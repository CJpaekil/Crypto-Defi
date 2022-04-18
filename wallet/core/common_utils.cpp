// Copyright 2019 The Beam Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "wallet/core/common_utils.h"
#include <numeric>

namespace beam::wallet
{

    bool ReadTreasury(ByteBuffer& bb, const std::string& sPath)
    {
        if (sPath.empty())
            return false;

        std::FStream f;
        if (!f.Open(sPath.c_str(), true))
            return false;

        size_t nSize = static_cast<size_t>(f.get_Remaining());
        if (!nSize)
            return false;

        bb.resize(f.get_Remaining());
        return f.read(&bb.front(), nSize) == nSize;
    }

Amount AccumulateCoinsSum(const std::vector<Coin>& vSelStd, const std::vector<ShieldedCoin>& vSelShielded)
{
    Amount sum = accumulate(vSelStd.begin(), vSelStd.end(), (Amount)0, [] (Amount sum, const Coin& c) {
        return sum + c.m_ID.m_Value;
    });

    sum = accumulate(vSelShielded.begin(), vSelShielded.end(), sum, [] (Amount sum, const ShieldedCoin& c) {
        return sum + c.m_CoinID.m_Value;
    });

    return sum;
}

Amount CalcCoinSelectionInfo2(Height h, const IWalletDB::Ptr& walletDB, Amount requestedSum, Asset::ID aid, Amount valFeeInputShielded, Amount& feesInvoluntary)
{
    std::vector<Coin> vStd;
    std::vector<ShieldedCoin> vShielded;

    uint32_t nMaxShielded = Rules::get().Shielded.MaxIns;
    if (aid)
        nMaxShielded /= 2; // leave some for std

    walletDB->selectCoins2(h, requestedSum, aid, vStd, vShielded, nMaxShielded, true);

    Amount ret = 0;
    for (const auto& c : vStd)
        ret += c.m_ID.m_Value;

    for (const auto& c : vShielded)
        ret += c.m_CoinID.m_Value;

    auto nInputsShielded = static_cast<uint32_t>(vShielded.size());
    feesInvoluntary = valFeeInputShielded * nInputsShielded;

    return ret;
}


void CoinsSelectionInfo::Calculate(Height h, const IWalletDB::Ptr& walletDB, bool isPushTx)
{
    auto& fs = Transaction::FeeSettings::get(h);

    m_minimalRawFee = fs.m_Kernel;
    if (isPushTx)
    {
        m_minimalExplicitFee = fs.get_DefaultShieldedOut();
        m_minimalRawFee += fs.m_ShieldedOutputTotal;
    }
    else
    {
        m_minimalExplicitFee = fs.get_DefaultStd();
        m_minimalRawFee += fs.m_Output;
    }

    m_isEnought = true;

    Amount valBeams = m_requestedSum;
    m_involuntaryFee = 0;

    if (m_assetID)
    {
        m_changeAsset = 0;
        m_selectedSumAsset = CalcCoinSelectionInfo2(h, walletDB, m_requestedSum, m_assetID, fs.m_ShieldedInputTotal, m_involuntaryFee);

        if (m_selectedSumAsset < m_requestedSum) {
            m_isEnought = false;
        }
        else {
            if (m_selectedSumAsset > m_requestedSum) {
                // change
                m_changeAsset = m_selectedSumAsset - m_requestedSum;
                m_minimalRawFee += fs.m_Output;
            }
        }

        valBeams = 0;
    }

    std::setmax(m_minimalExplicitFee, m_minimalRawFee);
    std::setmax(m_explicitFee, m_minimalExplicitFee);
    valBeams += m_explicitFee + m_involuntaryFee;

    m_changeBeam = 0;

    Amount feeInvoluntary2;
    m_selectedSumBeam = CalcCoinSelectionInfo2(h, walletDB, valBeams, 0, fs.m_ShieldedInputTotal, feeInvoluntary2);

    if (m_selectedSumBeam > valBeams + feeInvoluntary2) {

        // change output is necessary
        m_minimalRawFee += fs.m_Output;
        std::setmax(m_minimalExplicitFee, m_minimalRawFee);

        if (m_explicitFee < m_minimalExplicitFee)
        {
            // retry
            valBeams += m_minimalExplicitFee - m_explicitFee;
            m_explicitFee = m_minimalExplicitFee;

            m_selectedSumBeam = CalcCoinSelectionInfo2(h, walletDB, valBeams, 0, fs.m_ShieldedInputTotal, feeInvoluntary2);
        }
    }

    valBeams += feeInvoluntary2;
    m_involuntaryFee += feeInvoluntary2;

    if (m_selectedSumBeam < valBeams) {
        m_isEnought = false;
    }
    else {
        if (m_selectedSumBeam > valBeams) {
            m_changeBeam = m_selectedSumBeam - valBeams;
        }
    }

    if (!m_assetID)
    {
        m_selectedSumAsset = m_selectedSumBeam;
        m_changeAsset = m_changeBeam;
    }
}

Amount CoinsSelectionInfo::get_TotalFee() const
{
    return m_explicitFee + m_involuntaryFee;
}

Amount CoinsSelectionInfo::get_NettoValue() const
{
    Amount val = m_selectedSumAsset - m_changeAsset;
    if (m_assetID)
        return val;

    // subtract the fee
    Amount fees = get_TotalFee();

    return (val > fees) ? (val - fees) : 0;
}

}  // namespace beam::wallet
