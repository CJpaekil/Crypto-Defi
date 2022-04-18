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

#include "../wallet_model.h"
#include "apps_api_ui.h"

#include "wallet/api/i_wallet_api.h"
#include "wallet/core/contracts/i_shaders_manager.h"
#include "wallet/client/apps_api/apps_api.h"

class WebAPICreator {
    
public:
    WebAPICreator();

    void createApi(WalletModel::Ptr walletModel, const std::string& verWant, const std::string& verMin, const std::string& appName, const std::string& appUrl);
    bool apiSupported(const std::string& apiVersion) const;
    std::string generateAppID(const std::string& appName, const std::string& appUrl);
    
    void apiChanged();
    
    std::shared_ptr<AppsApiUI> _api;
};

