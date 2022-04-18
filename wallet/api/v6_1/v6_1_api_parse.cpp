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
#include "v6_1_api.h"
#include "utility/fsutils.h"

namespace beam::wallet
{
    std::pair<EvSubUnsub, IWalletApi::MethodInfo> V61Api::onParseEvSubUnsub(const JsonRpcId& id, const nlohmann::json& params)
    {
        std::set<std::string> allowed;
        allowed.insert("ev_sync_progress");
        allowed.insert("ev_system_state");
        allowed.insert("ev_assets_changed");
        allowed.insert("ev_addrs_changed");
        allowed.insert("ev_utxos_changed");
        allowed.insert("ev_txs_changed");
        allowed.insert("ev_connection_changed");

        bool found = false;
        for (auto it : params.items())
        {
            if(allowed.find(it.key()) == allowed.end())
            {
                std::string error = "The event '" + it.key() + "' is unknown.";
                throw jsonrpc_exception(ApiError::InvalidParamsJsonRpc, error);
            }
            else
            {
                found = true;
            }

            if (!getAppId().empty())
            {
                // Some events are not allowed for applications
                if (it.key() == "ev_utxos_changed" ||
                    it.key() == "ev_assets_changed")
                {
                    throw jsonrpc_exception(ApiError::NotAllowedError);
                }
            }
        }

        if (found == false)
        {
            throw jsonrpc_exception(ApiError::InvalidParamsJsonRpc, "Must subunsub at least one supported event");
        }

        // So here we are sure that at least one known event is present
        // Typecheck & get value
        EvSubUnsub message;
        message.syncProgress   = getOptionalParam<bool>(params, "ev_sync_progress");
        message.systemState    = getOptionalParam<bool>(params, "ev_system_state");
        message.assetChanged   = getOptionalParam<bool>(params, "ev_assets_changed");
        message.addrsChanged   = getOptionalParam<bool>(params, "ev_addrs_changed");
        message.utxosChanged   = getOptionalParam<bool>(params, "ev_utxos_changed");
        message.txsChanged     = getOptionalParam<bool>(params, "ev_txs_changed");
        message.connectChanged = getOptionalParam<bool>(params, "ev_connection_changed");

        return std::make_pair(message, MethodInfo());
    }

    void V61Api::getResponse(const JsonRpcId& id, const EvSubUnsub::Response& res, json& msg)
    {
        msg = json
        {
            {JsonRpcHeader, JsonRpcVersion},
            {"id", id},
            {"result", res.result}
        };
    }

    std::pair<GetVersion, IWalletApi::MethodInfo> V61Api::onParseGetVersion(const JsonRpcId& id, const nlohmann::json& params)
    {
        GetVersion message{};
        return std::make_pair(message, MethodInfo());
    }

    void V61Api::getResponse(const JsonRpcId& id, const GetVersion::Response& res, json& msg)
    {
        msg = json
        {
            {JsonRpcHeader, JsonRpcVersion},
            {"id", id},
            {"result",
                {
                    {"api_version",        res.apiVersion},
                    {"api_version_major",  res.apiVersionMajor},
                    {"api_version_minor",  res.apiVersionMinor},
                    {"beam_version",       res.beamVersion},
                    {"beam_version_major", res.beamVersionMajor},
                    {"beam_version_minor", res.beamVersionMinor},
                    {"beam_version_rev",   res.beamVersionRevision},
                    {"beam_commit_hash",   res.beamCommitHash},
                    {"beam_branch_name",   res.beamBranchName}
                }
            }
        };
    }

    std::pair<WalletStatusV61, IWalletApi::MethodInfo> V61Api::onParseWalletStatusV61(const JsonRpcId& id, const nlohmann::json& params)
    {
        WalletStatusV61 message{};
        message.nzOnly = getOptionalParam<bool>(params, "nz_totals");
        return std::make_pair(message, MethodInfo());
    }

    void V61Api::getResponse(const JsonRpcId& id, const WalletStatusV61::Response& res, json& msg)
    {
        msg = json
        {
            {JsonRpcHeader, JsonRpcVersion},
            {"id", id},
            {"result",
                {
                    {"current_height", res.currentHeight},
                    {"current_state_hash", to_hex(res.currentStateHash.m_pData, res.currentStateHash.nBytes)},
                    {"current_state_timestamp", res.currentStateTimestamp},
                    {"prev_state_hash", to_hex(res.prevStateHash.m_pData, res.prevStateHash.nBytes)},
                    {"is_in_sync", res.isInSync},
                }
            }
        };

        if (!getAppId().empty())
        {
            return;
        }

        auto& result = msg["result"];
        result["available"]  = res.available;
        result["receiving"]  = res.receiving;
        result["sending"]    =  res.sending;
        result["maturing"]   =  res.maturing;
        result["difficulty"] =  res.difficulty;

        if (res.totals)
        {
            for(const auto& it: res.totals->GetAllTotals())
            {
                const auto& totals = it.second;
                json jtotals;

                jtotals["asset_id"] = totals.AssetId;

                auto avail = totals.Avail; avail += totals.AvailShielded;
                jtotals["available_str"] = std::to_string(avail);
                jtotals["available_regular_str"] = std::to_string(totals.Avail);
                jtotals["available_mp_str"] = std::to_string(totals.AvailShielded);

                if (avail <= kMaxAllowedInt)
                {
                    jtotals["available"] = AmountBig::get_Lo(avail);
                }

                if (totals.Avail <= kMaxAllowedInt)
                {
                    jtotals["available_regular"] = AmountBig::get_Lo(totals.Avail);
                }

                if (totals.AvailShielded <= kMaxAllowedInt)
                {
                    jtotals["available_mp"] = AmountBig::get_Lo(totals.AvailShielded);
                }

                auto incoming = totals.Incoming; incoming += totals.IncomingShielded;
                jtotals["receiving_str"] = std::to_string(incoming);
                jtotals["receiving_regular_str"] = std::to_string(totals.Incoming);
                jtotals["receiving_mp_str"] = std::to_string(totals.IncomingShielded);

                if (incoming <= kMaxAllowedInt)
                {
                    jtotals["receiving"] = AmountBig::get_Lo(incoming);
                }

                if (totals.Incoming <= kMaxAllowedInt)
                {
                    jtotals["receiving_regular"] = AmountBig::get_Lo(totals.Incoming);
                }

                if (totals.IncomingShielded <= kMaxAllowedInt)
                {
                    jtotals["receiving_mp"] = AmountBig::get_Lo(totals.IncomingShielded);
                }

                auto outgoing = totals.Outgoing; outgoing += totals.OutgoingShielded;
                jtotals["sending_str"] = std::to_string(outgoing);
                jtotals["sending_regular_str"] = std::to_string(totals.Outgoing);
                jtotals["sending_mp_str"] = std::to_string(totals.OutgoingShielded);

                if (outgoing <= kMaxAllowedInt)
                {
                    jtotals["sending"] = AmountBig::get_Lo(outgoing);
                }

                if (totals.Outgoing <= kMaxAllowedInt)
                {
                    jtotals["sending_regular"] = AmountBig::get_Lo(totals.Outgoing);
                }

                if (totals.OutgoingShielded <= kMaxAllowedInt)
                {
                    jtotals["sending_mp"] = AmountBig::get_Lo(totals.OutgoingShielded);
                }

                auto maturing = totals.Maturing; maturing += totals.MaturingShielded;
                jtotals["maturing_str"] = std::to_string(maturing);
                jtotals["maturing_regular_str"] = std::to_string(totals.Maturing);
                jtotals["maturing_mp_str"] = std::to_string(totals.MaturingShielded);

                if (maturing <= kMaxAllowedInt)
                {
                    jtotals["maturing"] = AmountBig::get_Lo(maturing);
                }

                if (totals.Maturing <= kMaxAllowedInt)
                {
                    jtotals["maturing_regular"] = AmountBig::get_Lo(totals.Maturing);
                }

                if (totals.MaturingShielded <= kMaxAllowedInt)
                {
                    jtotals["maturing_mp"] = AmountBig::get_Lo(totals.MaturingShielded);
                }

                auto change = totals.ReceivingChange;
                jtotals["change_str"] = std::to_string(change);

                if (change <= kMaxAllowedInt)
                {
                    jtotals["change"] = AmountBig::get_Lo(change);
                }

                auto locked = totals.Maturing;
                locked += totals.MaturingShielded;
                locked += totals.ReceivingChange;
                jtotals["locked_str"] = std::to_string(locked);

                if (locked <= kMaxAllowedInt)
                {
                    jtotals["locked"] = AmountBig::get_Lo(locked);
                }

                result["totals"].push_back(jtotals);
            }
        }
    }

    std::pair<InvokeContractV61, IWalletApi::MethodInfo> V61Api::onParseInvokeContractV61(const JsonRpcId &id, const json &params)
    {
        InvokeContractV61 message;

        if(const auto contract = getOptionalParam<NonEmptyJsonArray>(params, "contract"))
        {
            const json& bytes = *contract;
            message.contract = bytes.get<std::vector<uint8_t>>();
        }
        else if(const auto fname = getOptionalParam<NonEmptyString>(params, "contract_file"))
        {
            fsutils::fread(*fname).swap(message.contract);
        }

        if (const auto args = getOptionalParam<NonEmptyString>(params, "args"))
        {
            message.args = *args;
        }

        if (const auto createTx = getOptionalParam<bool>(params, "create_tx"))
        {
            message.createTx = *createTx;
        }

        if (isApp() && message.createTx)
        {
            throw jsonrpc_exception(ApiError::NotAllowedError, "Applications must set create_tx to false and use process_contract_data");
        }

        if (const auto priority = getOptionalParam<uint32_t>(params, "priority"))
        {
            message.priority = *priority;
        }

        if (const auto unique = getOptionalParam<uint32_t>(params, "unique"))
        {
            message.unique = *unique;
        }

        return std::make_pair(message, MethodInfo());
    }

    void V61Api::getResponse(const JsonRpcId& id, const InvokeContractV61::Response& res, json& msg)
    {
        msg = nlohmann::json
        {
            {JsonRpcHeader, JsonRpcVersion},
            {"id", id},
            {"result",
                {
                    {"output", res.output ? *res.output : std::string("")}
                }
            }
        };

        if (res.txid)
        {
            msg["result"]["txid"] = std::to_string(*res.txid);
        }

        if (res.invokeData)
        {
            msg["result"]["raw_data"] = *res.invokeData;
        }
    }
}
