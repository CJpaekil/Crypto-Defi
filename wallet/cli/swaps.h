// Copyright 2020 The Beam Team
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

#pragma once

#include <boost/program_options.hpp>

#include "utility/cli/options.h"

#include "wallet/core/wallet_db.h"
#include "wallet/core/common.h"
#include "wallet/core/wallet.h"
#include "wallet/transactions/swaps/common.h"

namespace beam::wallet
{
    boost::optional<TxID> InitSwap(const po::variables_map& vm, const IWalletDB::Ptr& walletDB, Wallet& wallet);
    boost::optional<TxID> AcceptSwap(const po::variables_map& vm, const IWalletDB::Ptr& walletDB, Wallet& wallet);
    bool HasActiveSwapTx(const IWalletDB::Ptr& walletDB, AtomicSwapCoin swapCoin);
    Amount EstimateSwapFeerate(beam::wallet::AtomicSwapCoin swapCoin, IWalletDB::Ptr walletDB);
    Amount GetBalance(beam::wallet::AtomicSwapCoin swapCoin, IWalletDB::Ptr walletDB);
    int SetSwapSettings(const po::variables_map& vm, const IWalletDB::Ptr& walletDB, AtomicSwapCoin swapCoin);
    void ShowSwapSettings(const po::variables_map& vm, const IWalletDB::Ptr& walletDB, AtomicSwapCoin swapCoin);
} // beam::wallet