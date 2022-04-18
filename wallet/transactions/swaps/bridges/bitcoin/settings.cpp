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

#include "settings.h"
#include "bitcoin/bitcoin.hpp"

namespace beam::bitcoin
{
    boost::optional<Settings::ConnectionType> from_string(const std::string& value)
    {
        if (value == "core")
            return Settings::ConnectionType::Core;
        else if (value == "electrum")
            return Settings::ConnectionType::Electrum;
        else if (value == "none")
            return Settings::ConnectionType::None;

        return boost::optional<Settings::ConnectionType>{};
    }

    std::string to_string(Settings::ConnectionType connectionType)
    {
        switch (connectionType)
        {
        case Settings::ConnectionType::Core:
            return "core";
        case Settings::ConnectionType::Electrum:
            return "electrum";
        default:
            return "none";
        }
    }

    std::string BitcoinCoreSettings::generateAuthorization()
    {
        std::string userWithPass(m_userName + ":" + m_pass);
        libbitcoin::data_chunk t(userWithPass.begin(), userWithPass.end());
        return std::string("Basic " + libbitcoin::encode_base64(t));
    }

    BitcoinCoreSettings Settings::GetConnectionOptions() const
    {
        return m_connectionSettings;
    }

    bool Settings::IsCoreActivated() const
    {
        return GetConnectionOptions().IsInitialized() &&
               GetCurrentConnectionType() == Settings::ConnectionType::Core;
    }

    ElectrumSettings Settings::GetElectrumConnectionOptions() const
    {
        return m_electrumConnectionSettings;
    }

    bool Settings::IsElectrumActivated() const
    {
        return GetElectrumConnectionOptions().IsInitialized() &&
               GetCurrentConnectionType() == Settings::ConnectionType::Electrum;
    }

    Amount Settings::GetMinFeeRate() const
    {
        return m_minFeeRate;
    }

    Amount Settings::GetMaxFeeRate() const
    {
        return m_maxFeeRate;
    }

    uint16_t Settings::GetLockTxMinConfirmations() const
    {
        return m_lockTxMinConfirmations;
    }

    uint16_t Settings::GetWithdrawTxMinConfirmations() const
    {
        return m_withdrawTxMinConfirmations;
    }

    uint32_t Settings::GetLockTimeInBlocks() const
    {
        return m_lockTimeInBlocks;
    }

    bool Settings::IsInitialized() const
    {
        return m_connectionSettings.IsInitialized() || m_electrumConnectionSettings.IsInitialized();
    }

    bool Settings::IsActivated() const
    {
        return IsCoreActivated() || IsElectrumActivated();
    }

    Settings::ConnectionType Settings::GetCurrentConnectionType() const
    {
        return m_connectionType;
    }

    double Settings::GetBlocksPerHour() const
    {
        return m_blocksPerHour;
    }

    uint8_t Settings::GetAddressVersion() const
    {
        return m_addressVersion;
    }

    std::vector<std::string> Settings::GetGenesisBlockHashes() const
    {
        return m_genesisBlockHashes;
    }

    void Settings::SetConnectionOptions(const BitcoinCoreSettings& connectionSettings)
    {
        m_connectionSettings = connectionSettings;
    }

    void Settings::SetElectrumConnectionOptions(const ElectrumSettings& connectionSettings)
    {
        m_electrumConnectionSettings = connectionSettings;
    }

    void Settings::SetMinFeeRate(beam::Amount feeRate)
    {
        m_minFeeRate = feeRate;
    }

    void Settings::SetMaxFeeRate(beam::Amount feeRate)
    {
        m_maxFeeRate = feeRate;
    }

    void Settings::SetLockTxMinConfirmations(uint16_t txMinConfirmations)
    {
        m_lockTxMinConfirmations = txMinConfirmations;
    }

    void Settings::SetWithdrawTxMinConfirmations(uint16_t txMinConfirmations)
    {
        m_withdrawTxMinConfirmations = txMinConfirmations;
    }

    void Settings::SetLockTimeInBlocks(uint32_t lockTimeInBlocks)
    {
        m_lockTimeInBlocks = lockTimeInBlocks;
    }

    void Settings::ChangeConnectionType(Settings::ConnectionType type)
    {
        m_connectionType = type;
    }

    void Settings::SetBlocksPerHour(double blocksPerHour)
    {
        m_blocksPerHour = blocksPerHour;
    }

    void Settings::SetAddressVersion(uint8_t addressVersion)
    {
        m_addressVersion = addressVersion;
    }

    void Settings::SetGenesisBlockHashes(const std::vector<std::string>& genesisBlockHashes)
    {
        m_genesisBlockHashes = genesisBlockHashes;
    }

    void Settings::DisableElectrum()
    {
        m_isSupportedElectrum = false;
    }

    bool Settings::IsSupportedElectrum() const
    {
        return m_isSupportedElectrum;
    }
} // namespace beam::bitcoin
