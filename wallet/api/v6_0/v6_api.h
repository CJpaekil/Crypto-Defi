// Copyright 2018 The Beam Team
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

#include "wallet/api/base/api_base.h"
#include "wallet/core/wallet.h"
#include "wallet/core/wallet_db.h"
#include "v6_api_defs.h"
#include "wallet/core/contracts/i_shaders_manager.h"

namespace beam::wallet
{
    class V6Api: public ApiBase
    {
    public:
        // MUST BE SAFE TO CALL FROM ANY THREAD
        V6Api(IWalletApiHandler& handler, const ApiInitData& init);

        virtual IWalletDB::Ptr       getWalletDB() const;
        virtual Wallet::Ptr          getWallet() const;
        virtual ISwapsProvider::Ptr  getSwaps() const;
        virtual IShadersManager::Ptr getContracts() const;
        virtual Height               get_TipHeight() const;

        #ifdef BEAM_IPFS_SUPPORT
        IPFSService::Ptr getIPFS() const;
        #endif

        void assertWalletThread() const;
        void checkCAEnabled() const;
        bool getCAEnabled() const;

        V6_API_METHODS(BEAM_API_PARSE_FUNC)
        V6_API_METHODS(BEAM_API_RESPONSE_FUNC)
        V6_API_METHODS(BEAM_API_HANDLE_FUNC)

    protected:
        virtual bool allowedTx(const TxDescription& tx);
        virtual void fillAssetInfo(json& arr, const WalletAsset& info);
        virtual void fillAddresses(json& arr, const std::vector<WalletAddress>& items);
        virtual void fillCoins(json& arr, const std::vector<ApiCoin>& coins);
        virtual void fillTransactions(json& arr, const std::vector<Status::Response>& txs);

    private:
        void FillAddressData(const AddressData& data, WalletAddress& address);
        void doTxAlreadyExistsError(const JsonRpcId& id);

        template<typename T>
        void onHandleIssueConsume(bool issue, const JsonRpcId& id, T&& data);
        template<typename T>
        void setTxAssetParams(const JsonRpcId& id, TxParameters& tx, const T& data);

        void onHandleInvokeContractWithTX(const JsonRpcId &id, InvokeContract&& data);
        void onHandleInvokeContractNoTX(const JsonRpcId &id, InvokeContract&& data);

        bool checkTxAccessRights(const TxParameters&);
        void checkTxAccessRights(const TxParameters&, ApiError code, const std::string& errmsg);

        template<typename T>
        std::pair<T, IWalletApi::MethodInfo> onParseIssueConsume(bool issue, const JsonRpcId& id, const json& params);

        // If no fee read and no min fee provided this function calculates minimum fee itself
        Amount getBeamFeeParam(const json& params, const std::string& name, Amount feeMin) const;
        Amount getBeamFeeParam(const json& params, const std::string& name, bool hasShieldedOutputs = false) const;

        std::string getTokenType(TokenType type) const;

    private:
        // Do not access these directly, use getters
        IWalletDB::Ptr       _wdb;
        Wallet::Ptr          _wallet;
        ISwapsProvider::Ptr  _swaps;
        IShadersManager::Ptr _contracts;

        #ifdef BEAM_IPFS_SUPPORT
        IPFSService::Ptr _ipfs;
        #endif

        struct RequestHeaderMsg
            : public proto::FlyClient::RequestEnumHdrs
            , public proto::FlyClient::Request::IHandler
        {
            typedef boost::intrusive_ptr<RequestHeaderMsg> Ptr;
            ~RequestHeaderMsg() override = default;

            RequestHeaderMsg(JsonRpcId id, IWalletApi::WeakPtr guard, V6Api& wapi)
                : _id(std::move(id))
                , _guard(std::move(guard))
                , _wapi(wapi)
            {}

            void OnComplete(proto::FlyClient::Request&) override;

        private:
            JsonRpcId _id;
            IWalletApi::WeakPtr _guard;
            V6Api& _wapi;
        };

        std::map<TokenType, std::string> _ttypesMap;
    };
}
