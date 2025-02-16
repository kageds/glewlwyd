import React, { Component } from 'react';

import apiManager from '../lib/APIManager';
import messageDispatcher from '../lib/MessageDispatcher';
import Notification from '../lib/Notification';
import i18next from 'i18next';

import Confirm from '../Modal/Confirm';
import Message from '../Modal/Message';
import EditRecord from '../Modal/EditRecord';

import Navbar from './Navbar';
import Users from './Users';
import Clients from './Clients';
import Scopes from './Scopes';
import UserMod from './UserMod';
import ClientMod from './ClientMod';
import UserMiddlewareMod from './UserMiddlewareMod';
import SchemeMod from './SchemeMod';
import Plugin from './Plugin';
import ScopeEdit from './ScopeEdit';
import ModEdit from './ModEdit';
import PluginEdit from './PluginEdit';
import APIKey from './APIKey';
import MiscConfig from './MiscConfig';

class App extends Component {
  constructor(props) {
    super(props);

    this.state = {
      lang: i18next.language,
      config: props.config,
      passwordMinLength: props.config.PasswordMinLength||8,
      curNav: "users",
      loggedIn: false,
      users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
      curUser: false,
      clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
      curClient: false,
      scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
      curScope: false,
      apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
      curApiKey: false,
      confirmModal: {title: "", message: ""},
      messageModal: {title: "", label: "", message: []},
      editModal: {title: "", pattern: [], source: [], data: {}, callback: false, validateCallback: false, add: false},
      scopeModal: {title: "", data: {name: "", display_name: "", description: "", password_required: true, scheme: {}}, callback: false, add: false},
      curMod: false,
      modUsers: [],
      defaultModUsers: false,
      modUsersMiddleware: [],
      ModModal: {title: "", role: false, data: {}, types: [], add: false, callback: false},
      modClients: [],
      defaultModClients: false,
      modSchemes: [],
      plugins: [],
      PluginModal: {title: "", data: {}, types: [], add: false, callback: false},
      modTypes: {user: [], client: [], scheme: [], plugin: []},
      profileList: false,
      invalidCredentialMessage: false,
      savedRecord: false,
      savedIndex: -1,
      miscConfig: []
    }

    this.fetchApi = this.fetchApi.bind(this);

    this.fetchUsers = this.fetchUsers.bind(this);
    this.confirmDeleteUser = this.confirmDeleteUser.bind(this);
    this.confirmEditUser = this.confirmEditUser.bind(this);
    this.confirmAddUser = this.confirmAddUser.bind(this);
    this.validateUser = this.validateUser.bind(this);

    this.fetchClients = this.fetchClients.bind(this);
    this.confirmDeleteClient = this.confirmDeleteClient.bind(this);
    this.confirmEditClient = this.confirmEditClient.bind(this);
    this.confirmAddClient = this.confirmAddClient.bind(this);
    this.validateClient = this.validateClient.bind(this);

    this.fetchScopes = this.fetchScopes.bind(this);
    this.fetchAllScopes = this.fetchAllScopes.bind(this);
    this.confirmDeleteScope = this.confirmDeleteScope.bind(this);
    this.confirmEditScope = this.confirmEditScope.bind(this);
    this.confirmAddScope = this.confirmAddScope.bind(this);

    this.fetchModTypes = this.fetchModTypes.bind(this);
    this.fetchUserMods = this.fetchUserMods.bind(this);
    this.fetchUserMiddlewareMods = this.fetchUserMiddlewareMods.bind(this);
    this.fetchClientMods = this.fetchClientMods.bind(this);
    this.fetchSchemeMods = this.fetchSchemeMods.bind(this);
    this.fetchPlugins = this.fetchPlugins.bind(this);

    this.confirmAddUserMod = this.confirmAddUserMod.bind(this);
    this.confirmEditUserMod = this.confirmEditUserMod.bind(this);
    this.confirmDeleteUserMod = this.confirmDeleteUserMod.bind(this);

    this.confirmAddUserMiddlewareMod = this.confirmAddUserMiddlewareMod.bind(this);
    this.confirmEditUserMiddlewareMod = this.confirmEditUserMiddlewareMod.bind(this);
    this.confirmDeleteUserMiddlewareMod = this.confirmDeleteUserMiddlewareMod.bind(this);

    this.confirmAddClientMod = this.confirmAddClientMod.bind(this);
    this.confirmEditClientMod = this.confirmEditClientMod.bind(this);
    this.confirmDeleteClientMod = this.confirmDeleteClientMod.bind(this);

    this.confirmAddSchemeMod = this.confirmAddSchemeMod.bind(this);
    this.confirmEditSchemeMod = this.confirmEditSchemeMod.bind(this);
    this.confirmDeleteSchemeMod = this.confirmDeleteSchemeMod.bind(this);

    this.confirmAddPluginMod = this.confirmAddPluginMod.bind(this);
    this.confirmEditPluginMod = this.confirmEditPluginMod.bind(this);
    this.confirmDeletePluginMod = this.confirmDeletePluginMod.bind(this);

    this.addApiKey = this.addApiKey.bind(this);
    this.confirmDisableApiKey = this.confirmDisableApiKey.bind(this);

    messageDispatcher.subscribe('App', (message) => {
      if (message.type === 'nav') {
        this.setState({curNav: message.message});
      } else if (message.type === 'profile') {
        this.fetchApi();
      } else if (message.type === 'loggedIn') {
        this.setState({loggedIn: message.loggedIn}, () => {
          this.fetchApi();
        });
      } else if (message.type === 'lang') {
        this.setState({lang: i18next.language});
      } else if (message.type === 'reloadApp') {
        this.reloadApp();
      } else if (message.type === 'delete') {
        if (message.role === 'user') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-user-title", {user: message.user.name}),
            message: i18next.t("admin.confirm-delete-user", {username: message.user.username, name: message.user.name}),
            callback: this.confirmDeleteUser
          }
          this.setState({confirmModal: confirmModal, curUser: message.user}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'client') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-client-title", {client: message.client.name}),
            message: i18next.t("admin.confirm-delete-client", {clientId: message.client.client_id, name: message.client.name}),
            callback: this.confirmDeleteClient
          }
          this.setState({confirmModal: confirmModal, curClient: message.client}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'scope') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-scope-title", {scope: message.scope.name}),
            message: i18next.t("admin.confirm-delete-scope", {scope: message.scope.name, name: message.scope.display_name}),
            callback: this.confirmDeleteScope
          }
          this.setState({confirmModal: confirmModal, curScope: message.scope}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'userMod') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-mod-title", {mod: message.mod.display_name}),
            message: i18next.t("admin.confirm-delete-mod", {mod: message.mod.display_name}),
            callback: this.confirmDeleteUserMod
          }
          this.setState({confirmModal: confirmModal, curMod: message.mod}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'userMiddlewareMod') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-mod-title", {mod: message.mod.display_name}),
            message: i18next.t("admin.confirm-delete-mod", {mod: message.mod.display_name}),
            callback: this.confirmDeleteUserMiddlewareMod
          }
          this.setState({confirmModal: confirmModal, curMod: message.mod}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'clientMod') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-mod-title", {mod: message.mod.display_name}),
            message: i18next.t("admin.confirm-delete-mod", {mod: message.mod.display_name}),
            callback: this.confirmDeleteClientMod
          }
          this.setState({confirmModal: confirmModal, curMod: message.mod}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'schemeMod') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-mod-title", {mod: message.mod.display_name}),
            message: i18next.t("admin.confirm-delete-mod", {mod: message.mod.display_name}),
            callback: this.confirmDeleteSchemeMod
          }
          this.setState({confirmModal: confirmModal, curMod: message.mod}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'plugin') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-mod-title", {mod: message.mod.display_name}),
            message: i18next.t("admin.confirm-delete-mod", {mod: message.mod.display_name}),
            callback: this.confirmDeletePluginMod
          }
          this.setState({confirmModal: confirmModal, curMod: message.mod}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'apiKey') {
          var confirmModal = {
            title: i18next.t("admin.confirm-delete-api-key-title"),
            message: i18next.t("admin.confirm-delete-api-key"),
            callback: this.confirmDisableApiKey
          }
          this.setState({confirmModal: confirmModal, curApiKey: message.apiKey}, () => {
            $("#confirmModal").modal({keyboard: false, show: true});
          });
        }
      } else if (message.type === 'edit') {
        if (message.role === 'user') {
          var editModal = {
            title: i18next.t("admin.edit-user-title", {user: message.user.name}),
            pattern: this.state.config.pattern.user,
            source: this.state.modUsers,
            data: message.user,
            callback: this.confirmEditUser,
            validateCallback: this.validateUser
          }
          this.setState({editModal: editModal, savedRecord: JSON.stringify(message.user), savedIndex: message.index}, () => {
            $("#editRecordModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'client') {
          var editModal = {
            title: i18next.t("admin.edit-client-title", {client: message.client.name}),
            pattern: this.state.config.pattern.client,
            data: message.client,
            source: this.state.modClients,
            callback: this.confirmEditClient,
            validateCallback: this.validateClient
          }
          this.setState({editModal: editModal, savedRecord: JSON.stringify(message.client), savedIndex: message.index}, () => {
            $("#editRecordModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'scope') {
          var scopeModal = {
            title: i18next.t("admin.edit-scope-title", {scope: message.scope.scope}),
            data: message.scope,
            callback: this.confirmEditScope
          }
          this.setState({scopeModal: scopeModal, savedRecord: JSON.stringify(message.scope), savedIndex: message.index}, () => {
            $("#editScopeModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'userMod') {
          var ModModal = {
            title: i18next.t("admin.edit-mod-title", {mod: message.mod.display_name}),
            data: message.mod,
            role: "user",
            types: this.state.modTypes.user,
            callback: this.confirmEditUserMod
          }
          this.setState({ModModal: ModModal, savedRecord: JSON.stringify(message.mod), savedIndex: message.index}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'userMiddlewareMod') {
          var ModModal = {
            title: i18next.t("admin.edit-mod-title", {mod: message.mod.display_name}),
            data: message.mod,
            role: "userMiddleware",
            types: this.state.modTypes.user_middleware,
            callback: this.confirmEditUserMiddlewareMod
          }
          this.setState({ModModal: ModModal, savedRecord: JSON.stringify(message.mod), savedIndex: message.index}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'clientMod') {
          var ModModal = {
            title: i18next.t("admin.edit-mod-title", {mod: message.mod.display_name}),
            data: message.mod,
            types: this.state.modTypes.client,
            role: "client",
            callback: this.confirmEditClientMod
          }
          this.setState({ModModal: ModModal, savedRecord: JSON.stringify(message.mod), savedIndex: message.index}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'schemeMod') {
          var ModModal = {
            title: i18next.t("admin.edit-mod-title", {mod: message.mod.display_name}),
            data: message.mod,
            types: this.state.modTypes.scheme,
            role: "scheme",
            callback: this.confirmEditSchemeMod
          }
          this.setState({ModModal: ModModal, savedRecord: JSON.stringify(message.mod), savedIndex: message.index}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'plugin') {
          var PluginModal = {
            title: i18next.t("admin.edit-mod-title", {mod: message.mod.display_name}),
            data: message.mod,
            types: this.state.modTypes.plugin,
            callback: this.confirmEditPluginMod
          }
          this.setState({PluginModal: PluginModal, savedRecord: JSON.stringify(message.mod), savedIndex: message.index}, () => {
            $("#editPluginModal").modal({keyboard: false, show: true});
          });
        }
      } else if (message.type === 'add') {
        if (message.role === 'user') {
          var editModal = {
            title: i18next.t("admin.add-user-title"),
            pattern: this.state.config.pattern.user,
            source: this.state.modUsers,
            defaultSource: this.state.defaultModUsers,
            data: {username: "", name: "", password: "", email: "", enabled: true, scope: []},
            callback: this.confirmAddUser,
            validateCallback: this.validateUser,
            add: true
          }
          this.setState({editModal: editModal}, () => {
            $("#editRecordModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'client') {
          var editModal = {
            title: i18next.t("admin.add-client-title"),
            pattern: this.state.config.pattern.client,
            source: this.state.modClients,
            defaultSource: this.state.defaultModClients,
            data: {client_id: "", confidential: false, client_secret: "", enabled: true, name: "", password: "", redirect_uri: [], scope: [], token_endpoint_auth_method: ["client_secret_basic"], authorization_type: ["code", "refresh_token"]},
            callback: this.confirmAddClient,
            validateCallback: this.validateClient,
            add: true
          }
          this.setState({editModal: editModal}, () => {
            $("#editRecordModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'scope') {
          var scopeModal = {
            title: i18next.t("admin.add-scope-title"),
            data: {name: "", display_name: "", description: "", password_required: true, password_max_age: 0, scheme: {}, schemeRequired: {}},
            callback: this.confirmAddScope,
            add: true
          }
          this.setState({scopeModal: scopeModal}, () => {
            $("#editScopeModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'userMod') {
          var ModModal = {
            title: i18next.t("admin.add-mod-title"),
            data: {order_rank: this.state.modUsers.length, parameters: {}},
            types: this.state.modTypes.user,
            role: "user",
            callback: this.confirmAddUserMod,
            add: true
          }
          this.setState({ModModal: ModModal}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'userMiddlewareMod') {
          var ModModal = {
            title: i18next.t("admin.add-mod-title"),
            data: {order_rank: this.state.modUsersMiddleware.length, parameters: {}},
            types: this.state.modTypes.user_middleware,
            role: "userMiddleware",
            callback: this.confirmAddUserMiddlewareMod,
            add: true
          }
          this.setState({ModModal: ModModal}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'clientMod') {
          var ModModal = {
            title: i18next.t("admin.add-mod-title"),
            data: {order_rank: this.state.modClients.length, parameters: {}},
            types: this.state.modTypes.client,
            role: "client",
            callback: this.confirmAddClientMod,
            add: true
          }
          this.setState({ModModal: ModModal}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'schemeMod') {
          var ModModal = {
            title: i18next.t("admin.add-mod-title"),
            data: {parameters: {}},
            types: this.state.modTypes.scheme,
            role: "scheme",
            callback: this.confirmAddSchemeMod,
            add: true
          }
          this.setState({ModModal: ModModal}, () => {
            $("#editModModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'plugin') {
          var PluginModal = {
            title: i18next.t("admin.add-mod-title"),
            data: {parameters: {}},
            types: this.state.modTypes.plugin,
            callback: this.confirmAddPluginMod,
            add: true
          }
          this.setState({PluginModal: PluginModal}, () => {
            $("#editPluginModal").modal({keyboard: false, show: true});
          });
        } else if (message.role === 'apiKey') {
          this.addApiKey();
        }
      } else if (message.type === 'swap') {
        if (message.role === 'userMod') {
          apiManager.glewlwydRequest("/mod/user/" + encodeURIComponent(message.mod.name), "PUT", message.mod)
          .then(() => {
            return apiManager.glewlwydRequest("/mod/user/" + encodeURIComponent(message.previousMod.name), "PUT", message.previousMod)
            .fail((err) => {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
              if (err.status !== 401) {
                messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
              } else {
                this.setState({
                  loggedIn: false,
                  modTypes: {user: [], client: [], scheme: [], plugin: []},
                  users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  modUsers: [],
                  modUsersMiddleware: [],
                  modClients: [],
                  modSchemes: [],
                  plugins: [],
                  invalidCredentialMessage: true
                });
              }
            })
          })
          .fail((err) => {
            if (err.status !== 401) {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
            } else {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
              this.setState({
                loggedIn: false,
                modTypes: {user: [], client: [], scheme: [], plugin: []},
                users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                modUsers: [],
                modUsersMiddleware: [],
                modClients: [],
                modSchemes: [],
                plugins: [],
                invalidCredentialMessage: true
              });
            }
          })
          .always(() => {
            this.fetchUserMods();
            this.fetchUsers();
          });
        } else if (message.role === 'userMiddlewareMod') {
          apiManager.glewlwydRequest("/mod/user_middleware/" + encodeURIComponent(message.mod.name), "PUT", message.mod)
          .then(() => {
            return apiManager.glewlwydRequest("/mod/user_middleware/" + encodeURIComponent(message.previousMod.name), "PUT", message.previousMod)
            .fail((err) => {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
              if (err.status !== 401) {
                messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
              } else {
                this.setState({
                  loggedIn: false,
                  modTypes: {user: [], client: [], scheme: [], plugin: []},
                  users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  modUsers: [],
                  modUsersMiddleware: [],
                  modClients: [],
                  modSchemes: [],
                  plugins: [],
                  invalidCredentialMessage: true
                });
              }
            })
          })
          .fail((err) => {
            if (err.status !== 401) {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
            } else {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
              this.setState({
                loggedIn: false,
                modTypes: {user: [], client: [], scheme: [], plugin: []},
                users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                modUsers: [],
                modUsersMiddleware: [],
                modClients: [],
                modSchemes: [],
                plugins: [],
                invalidCredentialMessage: true
              });
            }
          })
          .always(() => {
            this.fetchUserMiddlewareMods();
            this.fetchUsers();
          });
        } else if (message.role === 'clientMod') {
          apiManager.glewlwydRequest("/mod/client/" + encodeURIComponent(message.mod.name), "PUT", message.mod)
          .then(() => {
            return apiManager.glewlwydRequest("/mod/client/" + encodeURIComponent(message.previousMod.name), "PUT", message.previousMod)
            .fail((err) => {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
              if (err.status !== 401) {
                messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
              } else {
                this.setState({
                  loggedIn: false,
                  modTypes: {user: [], client: [], scheme: [], plugin: []},
                  users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                  modUsers: [],
                  modUsersMiddleware: [],
                  modClients: [],
                  modSchemes: [],
                  plugins: [],
                  invalidCredentialMessage: true
                });
              }
            });
          })
          .fail((err) => {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
            if (err.status !== 401) {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
            } else {
              this.setState({
                loggedIn: false,
                modTypes: {user: [], client: [], scheme: [], plugin: []},
                users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                modUsers: [],
                modUsersMiddleware: [],
                modClients: [],
                modSchemes: [],
                plugins: [],
                invalidCredentialMessage: true
              });
            }
          })
          .always(() => {
            this.fetchClientMods()
            this.fetchClients();
          });
        }
      } else if (message.type === 'search') {
        if (message.role === 'user') {
          var users = this.state.users;
          users.searchPattern = message.searchPattern;
          users.offset = message.offset;
          users.limit = message.limit;
          this.setState({users: users}, () => {
            this.fetchUsers();
          });
        } else if (message.role === 'client') {
          var clients = this.state.clients;
          clients.searchPattern = message.searchPattern;
          clients.offset = message.offset;
          clients.limit = message.limit;
          this.setState({clients: clients}, () => {
            this.fetchClients();
          });
        } else if (message.role === 'scope') {
          var scopes = this.state.scopes;
          scopes.searchPattern = message.searchPattern;
          scopes.offset = message.offset;
          scopes.limit = message.limit;
          this.setState({scopes: scopes}, () => {
            this.fetchScopes();
          });
        }
      } else if (message.type === 'refresh') {
        if (message.role === 'schemeMod') {
          this.fetchSchemeMods();
        } else if (message.role === 'userMod') {
          this.fetchUserMods();
        } else if (message.role === 'userMiddlewareMod') {
          this.fetchUserMiddlewareMods();
        } else if (message.role === 'clientMod') {
          this.fetchClientMods();
        } else if (message.role === 'pluginMod') {
          this.fetchPlugins();
        }
      } else if (message.type === 'miscConfig') {
        this.fetchMiscConfig();
      }
    });

    if (this.state.config) {
      this.fetchApi();
    }
  }

  reloadApp() {
    this.fetchModTypes();
    this.fetchUserMods();
    this.fetchUserMiddlewareMods();
    this.fetchClientMods();
    this.fetchSchemeMods();
    this.fetchPlugins();
    this.fetchApiKeys();
    this.fetchUsers();
    this.fetchClients();
    this.fetchAllScopes();
    this.fetchMiscConfig();
    messageDispatcher.sendMessage('Notification', {type: "info", message: i18next.t("admin.data-reloaded")});
  }

  fetchApi() {
    apiManager.glewlwydRequest("/profile_list")
    .then((res) => {
      this.setState({profileList: res}, () => {
        this.fetchUsers()
        .then(() => {
          this.setState({invalidCredentialMessage: false}, () => {
            this.fetchClients()
            .always(() => {
              this.fetchScopes();
            });
            this.fetchModTypes();
            this.fetchUserMods();
            this.fetchUserMiddlewareMods();
            this.fetchClientMods();
            this.fetchSchemeMods();
            this.fetchPlugins();
            this.fetchAllScopes();
            this.fetchApiKeys();
            this.fetchMiscConfig();
          });
        })
        .fail((error) => {
          this.setState({
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        });
      });
    })
    .fail((error) => {
      this.setState({invalidCredentialMessage: true}, () => {
        this.setState({
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          profileList: []
        }, () => {
          if (error.status === 401) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          } else {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
          }
        });
      });
    });
  }

  fetchUsers() {
    return apiManager.glewlwydRequest("/user?offset=" + this.state.users.offset + "&limit=" + this.state.users.limit + (this.state.users.searchPattern?"&pattern="+this.state.users.searchPattern:""))
    .then((users) => {
      var curUsers = this.state.users;
      curUsers.list = users;
      curUsers.pattern = this.state.config.pattern.user;
      this.setState({users: curUsers, loggedIn: true});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchClients() {
    return apiManager.glewlwydRequest("/client?offset=" + this.state.clients.offset + "&limit=" + this.state.clients.limit + (this.state.clients.searchPattern?"&pattern="+this.state.clients.searchPattern:""))
    .then((clients) => {
      var curClients = this.state.clients;
      curClients.list = clients;
      curClients.pattern = this.state.config.pattern.client;
      this.setState({clients: curClients});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchScopes() {
    return apiManager.glewlwydRequest("/scope?offset=" + this.state.scopes.offset + "&limit=" + this.state.scopes.limit + (this.state.scopes.searchPattern?"&pattern="+this.state.scopes.searchPattern:""))
    .then((scopes) => {
      var curScopes = this.state.scopes;
      curScopes.list = scopes;
      return this.setState({scopes: curScopes});
    })
    .fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        return this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchAllScopes() {
    return apiManager.glewlwydRequest("/scope?limit=0")
    .then((scopes) => {
      var scopeList = [];
      var users = this.state.users;
      var clients = this.state.clients;
      var config = this.state.config;
      scopes.forEach((scope) => {
        scopeList.push(scope.name);
      });
      users.pattern.forEach((pat) => {
        if (pat.name === "scope") {
          pat.listElements = scopeList;
        }
      });
      clients.pattern.forEach((pat) => {
        if (pat.name === "scope") {
          pat.listElements = scopeList;
        }
      });
      config.scopes = scopes;
      return this.setState({users: users, clients: clients, config: config});
    });
  }

  fetchUserMods () {
    return apiManager.glewlwydRequest("/mod/user")
    .then((modUsers) => {
      let defaultModUsers = false;
      modUsers.forEach((mod) => {
        if (!mod.readonly && !defaultModUsers) {
          defaultModUsers = mod.name;
        }
      });
      this.setState({modUsers: modUsers, defaultModUsers: defaultModUsers});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchUserMiddlewareMods () {
    return apiManager.glewlwydRequest("/mod/user_middleware")
    .then((modUsersMiddleware) => {
      this.setState({modUsersMiddleware: modUsersMiddleware});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchModTypes () {
    return apiManager.glewlwydRequest("/mod/type")
    .then((modTypes) => {
      this.setState({modTypes: modTypes});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchClientMods () {
    return apiManager.glewlwydRequest("/mod/client")
    .then((modClients) => {
      let defaultModClients = false;
      modClients.forEach((mod) => {
        if (!mod.readonly && !defaultModClients) {
          defaultModClients = mod.name;
        }
      });
      this.setState({modClients: modClients, defaultModClients: defaultModClients});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchSchemeMods () {
    return apiManager.glewlwydRequest("/mod/scheme")
    .then((modSchemes) => {
      this.setState({modSchemes: modSchemes});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchPlugins () {
    return apiManager.glewlwydRequest("/mod/plugin")
    .then((plugins) => {
      this.setState({plugins: plugins});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchApiKeys () {
    return apiManager.glewlwydRequest("/key?offset=" + this.state.apiKeys.offset + "&limit=" + this.state.apiKeys.limit + (this.state.apiKeys.searchPattern?"&pattern="+this.state.apiKeys.searchPattern:""))
    .then((apiKeys) => {
      var curApiKeys = this.state.apiKeys;
      curApiKeys.list = apiKeys;
      curApiKeys.pattern = this.state.config.pattern.user;
      this.setState({apiKeys: curApiKeys, loggedIn: true});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  fetchMiscConfig () {
    return apiManager.glewlwydRequest("/misc")
    .then((miscConfig) => {
      this.setState({miscConfig: miscConfig});
    }).fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-fetch")});
      } else {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    });
  }

  confirmDeleteUser(result) {
    if (result) {
      apiManager.glewlwydRequest("/user/" + encodeURIComponent(this.state.curUser.username), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-user")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-user")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUsers()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  confirmDeleteClient(result) {
    if (result) {
      apiManager.glewlwydRequest("/client/" + encodeURIComponent(this.state.curClient.client_id), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-client")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-client")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchClients()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  confirmDeleteScope(result) {
    if (result) {
      apiManager.glewlwydRequest("/scope/" + encodeURIComponent(this.state.curScope.name), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-scope")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-scope")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchAllScopes();
        this.fetchScopes()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  confirmEditUser(result, user) {
    if (result) {
      apiManager.glewlwydRequest("/user/" + encodeURIComponent(user.username), "PUT", user)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-set-user")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-set-user")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUsers()
        .always(() => {
          this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, savedRecord: false, savedIndex: -1}}, () => {
            $("#editRecordModal").modal("hide");
          });
        });
      });
    } else {
      var users = this.state.users;
      users.list[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, users: users, savedRecord: false, savedIndex: -1}}, () => {
        $("#editRecordModal").modal("hide");
      });
    }
  }

  confirmEditClient(result, client) {
    if (result) {
      apiManager.glewlwydRequest("/client/" + encodeURIComponent(client.client_id), "PUT", client)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-set-client")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-set-client")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchClients()
        .always(() => {
          this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, savedRecord: false, savedIndex: -1}}, () => {
            $("#editRecordModal").modal("hide");
          });
        });
      });
    } else {
      var clients = this.state.clients;
      clients.list[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, clients: clients, savedRecord: false, savedIndex: -1}}, () => {
        $("#editRecordModal").modal("hide");
      });
    }
  }

  confirmEditScope(result, scope) {
    if (result) {
      apiManager.glewlwydRequest("/scope/" + encodeURIComponent(scope.name), "PUT", scope)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-set-scope")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-set-scope")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchAllScopes();
        this.fetchScopes()
        .always(() => {
          this.setState({scopeModal: {data: {}, callback: false, savedRecord: false, savedIndex: -1}}, () => {
            $("#editScopeModal").modal("hide");
          });
        });
      });
    } else {
      var scopes = this.state.scopes;
      scopes.list[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({scopeModal: {data: {}, callback: false, scopes: scopes, savedRecord: false, savedIndex: -1}}, () => {
        $("#editScopeModal").modal("hide");
      });
    }
  }

  confirmAddUser(result, user) {
    if (result) {
      var source = (user.source?"?source="+user.source:"");
      apiManager.glewlwydRequest("/user/" + source, "POST", user)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-user")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-user")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUsers()
        .always(() => {
          this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, add: false}}, () => {
            $("#editRecordModal").modal("hide");
          });
        });
      });
    } else {
      this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, add: false}}, () => {
        $("#editRecordModal").modal("hide");
      });
    }
  }

  confirmAddClient(result, client) {
    if (result) {
      var source = (client.source?"?source="+client.source:"");
      apiManager.glewlwydRequest("/client/" + source, "POST", client)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-client")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-client")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchClients()
        .always(() => {
          this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, add: false}}, () => {
            $("#editRecordModal").modal("hide");
          });
        });
      });
    } else {
      this.setState({editModal: {title: "", pattern: [], source: [], data: {}, callback: false, add: false}}, () => {
        $("#editRecordModal").modal("hide");
      });
    }
  }

  confirmAddScope(result, scope) {
    if (result) {
      apiManager.glewlwydRequest("/scope/", "POST", scope)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-scope")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-scope")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchAllScopes();
        this.fetchScopes()
        .always(() => {
          this.setState({scopeModal: {data: {}, callback: false}}, () => {
            $("#editScopeModal").modal("hide");
          });
        });
      });
    } else {
      $("#editScopeModal").modal("hide");
    }
  }

  validateUser(user, confirmData, add, cb) {
    var result = true, data = {};
    if (add) {
      if (!Array.isArray(user.password) && (user.password !== undefined || confirmData.password !== undefined)) {
        if (user.password !== confirmData.password) {
          result = false;
          data["password"] = i18next.t("admin.user-password-error-match");
        } else if (user.password.length && user.password.length < this.state.passwordMinLength) {
          result = false;
          data["password"] = i18next.t("admin.user-password-error-invalid", {minLength: this.state.passwordMinLength});
        }
      }
      if (!user.username) {
        result = false;
        data["username"] = i18next.t("admin.user-username-mandatory");
        cb(result, data);
      } else {
        apiManager.glewlwydRequest("/user/" + encodeURIComponent(user.username))
        .then(() => {
          result = false;
          data["username"] = i18next.t("admin.user-username-exists");
        })
        .always(() => {
          cb(result, data);
        });
      }
    } else {
      if (!Array.isArray(user.password) && (user.password !== undefined || confirmData.password !== undefined)) {
        if (user.password || confirmData.password) {
          if (user.password !== confirmData.password) {
            result = false;
            data["password"] = i18next.t("admin.user-password-error-match");
          } else if (user.password.length && user.password.length < this.state.passwordMinLength) {
            result = false;
            data["password"] = i18next.t("admin.user-password-error-invalid", {minLength: this.state.passwordMinLength});
          }
        }
      }
      cb(result, data);
    }
  }

  validateClient(client, confirmData, add, cb) {
    var result = true, data = {};
    if (client.confidential) {
      if (client.password || confirmData.password) {
        if (client.password || confirmData.password) {
          if (client.password !== confirmData.password) {
            result = false;
            data["password"] = i18next.t("admin.user-password-error-match");
          } else if (client.password.length && client.password.length < this.state.passwordMinLength) {
            result = false;
            data["password"] = i18next.t("admin.user-password-error-invalid", {minLength: this.state.passwordMinLength});
          }
        } else if (!client.password && add) {
          result = false;
          data["password"] = i18next.t("admin.user-password-mandatory");
        }
      }
    }
    if (add) {
      if (!client.client_id) {
        result = false;
        data["client_id"] = i18next.t("admin.client-client-id-mandatory");
        cb(result, data);
      } else {
        apiManager.glewlwydRequest("/client/" + encodeURIComponent(client.client_id))
        .then(() => {
          result = false;
          data["client_id"] = i18next.t("admin.client-client-id-exists");
        })
        .always(() => {
          cb(result, data);
        });
      }
    } else {
      cb(result, data);
    }
  }

  confirmAddUserMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/user/", "POST", mod)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-mod")});
        if (err.status === 400) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
        } else if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUserMods()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: []}}, () => {
            $("#editModModal").modal("hide");
            this.fetchUsers();
          });
        });
      });
    } else {
      $("#editModModal").modal("hide");
    }
  }

  confirmEditUserMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/user/" + encodeURIComponent(mod.name), "PUT", mod)
      .then(() => {
        apiManager.glewlwydRequest("/mod/user/" + encodeURIComponent(mod.name) + "/reset/", "PUT")
        .then(() => {
          messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-edit-mod")});
        })
        .fail((err) => {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
          if (err.status === 400) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
          } else if (err.status !== 401) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
          } else {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
            this.setState({
              loggedIn: false,
              modTypes: {user: [], client: [], scheme: [], plugin: []},
              users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              modUsers: [],
              modUsersMiddleware: [],
              modClients: [],
              modSchemes: [],
              plugins: [],
              invalidCredentialMessage: true
            });
          }
        })
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUserMods()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: [], savedRecord: false, savedIndex: -1}}, () => {
            $("#editModModal").modal("hide");
            this.fetchUsers();
          });
        });
      });
    } else {
      var modUsers = this.state.modUsers;
      modUsers[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({modUsers: modUsers, savedRecord: false, savedIndex: -1}, () => {
        $("#editModModal").modal("hide");
      });
    }
  }

  confirmDeleteUserMod(result) {
    if (result) {
      apiManager.glewlwydRequest("/mod/user/" + encodeURIComponent(this.state.curMod.name), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUserMods()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
            this.fetchUsers();
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  confirmAddUserMiddlewareMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/user_middleware/", "POST", mod)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-mod")});
        if (err.status === 400) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
        } else if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUserMiddlewareMods()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: []}}, () => {
            $("#editModModal").modal("hide");
            this.fetchUsers();
          });
        });
      });
    } else {
      $("#editModModal").modal("hide");
    }
  }

  confirmEditUserMiddlewareMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/user_middleware/" + encodeURIComponent(mod.name), "PUT", mod)
      .then(() => {
        apiManager.glewlwydRequest("/mod/user_middleware/" + encodeURIComponent(mod.name) + "/reset/", "PUT")
        .then(() => {
          messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-edit-mod")});
        })
        .fail((err) => {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
          if (err.status === 400) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
          } else if (err.status !== 401) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
          } else {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
            this.setState({
              loggedIn: false,
              modTypes: {user: [], client: [], scheme: [], plugin: []},
              users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              modUsers: [],
              modUsersMiddleware: [],
              modClients: [],
              modSchemes: [],
              plugins: [],
              invalidCredentialMessage: true
            });
          }
        })
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUserMiddlewareMods()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: [], savedRecord: false, savedIndex: -1}}, () => {
            $("#editModModal").modal("hide");
            this.fetchUsers();
          });
        });
      });
    } else {
      var modUsers = this.state.modUsers;
      modUsers[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({modUsers: modUsers, savedRecord: false, savedIndex: -1}, () => {
        $("#editModModal").modal("hide");
      });
    }
  }

  confirmDeleteUserMiddlewareMod(result) {
    if (result) {
      apiManager.glewlwydRequest("/mod/user_middleware/" + encodeURIComponent(this.state.curMod.name), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchUserMiddlewareMods()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
            this.fetchUsers();
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  confirmAddClientMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/client/", "POST", mod)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-mod")});
        if (err.status === 400) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
        } else if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchClientMods()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: []}}, () => {
            $("#editModModal").modal("hide");
            this.fetchClients();
          });
        });
      });
    } else {
      $("#editModModal").modal("hide");
    }
  }

  confirmEditClientMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/client/" + encodeURIComponent(mod.name), "PUT", mod)
      .then(() => {
        apiManager.glewlwydRequest("/mod/client/" + encodeURIComponent(mod.name) + "/reset/", "PUT")
        .then(() => {
          messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-edit-mod")});
        })
        .fail((err) => {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
          if (err.status === 400) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
          } else if (err.status !== 401) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
          } else {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
            this.setState({
              loggedIn: false,
              modTypes: {user: [], client: [], scheme: [], plugin: []},
              users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              modUsers: [],
              modUsersMiddleware: [],
              modClients: [],
              modSchemes: [],
              plugins: [],
              invalidCredentialMessage: true
            });
          }
        })
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchClientMods()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: []}, savedRecord: false, savedIndex: -1}, () => {
            $("#editModModal").modal("hide");
            this.fetchClients();
          });
        });
      });
    } else {
      var modClients = this.state.modClients;
      modClients[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({modClients: modClients, savedRecord: false, savedIndex: -1}, () => {
        $("#editModModal").modal("hide");
      });
    }
  }

  confirmDeleteClientMod(result) {
    if (result) {
      apiManager.glewlwydRequest("/mod/client/" + encodeURIComponent(this.state.curMod.name), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchClientMods()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
            this.fetchClients();
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  confirmAddSchemeMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/scheme/", "POST", mod)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-mod")});
        if (err.status === 400) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
        } else if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchSchemeMods()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: []}}, () => {
            $("#editModModal").modal("hide");
          });
        });
      });
    } else {
      $("#editModModal").modal("hide");
    }
  }

  confirmEditSchemeMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/scheme/" + encodeURIComponent(mod.name), "PUT", mod)
      .then(() => {
          apiManager.glewlwydRequest("/mod/scheme/" + encodeURIComponent(mod.name) + "/reset/", "PUT")
          .then(() => {
            messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-edit-mod")});
          })
          .fail((err) => {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
            if (err.status === 400) {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
            } else if (err.status !== 401) {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
            } else {
              messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
              this.setState({
                loggedIn: false,
                modTypes: {user: [], client: [], scheme: [], plugin: []},
                users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
                modUsers: [],
                modUsersMiddleware: [],
                modClients: [],
                modSchemes: [],
                plugins: [],
                invalidCredentialMessage: true
              });
            }
          })
          .always(() => {
            this.fetchSchemeMods()
          });
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.setState({ModModal: {data: {}, callback: false, types: []}, savedRecord: false, savedIndex: -1}, () => {
          $("#editModModal").modal("hide");
        });
      });
    } else {
      var modSchemes = this.state.modSchemes;
      modSchemes[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({modSchemes: modSchemes, savedRecord: false, savedIndex: -1}, () => {
        $("#editModModal").modal("hide");
      });
    }
  }

  confirmDeleteSchemeMod(result) {
    if (result) {
      apiManager.glewlwydRequest("/mod/scheme/" + encodeURIComponent(this.state.curMod.name), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchSchemeMods()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  confirmAddPluginMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/plugin/", "POST", mod)
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-add-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-add-mod")});
        if (err.status === 400) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
        } else if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchPlugins()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: []}}, () => {
            $("#editPluginModal").modal("hide");
          });
        });
      });
    } else {
      $("#editPluginModal").modal("hide");
    }
  }

  confirmEditPluginMod(result, mod) {
    if (result) {
      apiManager.glewlwydRequest("/mod/plugin/" + encodeURIComponent(mod.name), "PUT", mod)
      .then(() => {
        apiManager.glewlwydRequest("/mod/plugin/" + encodeURIComponent(mod.name) + "/reset/", "PUT")
        .then(() => {
          messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-edit-mod")});
        })
        .fail((err) => {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
          if (err.status !== 401) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
          } else if (err.status === 400) {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: JSON.stringify(err.responseJSON)});
          } else {
            messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
            this.setState({
              loggedIn: false,
              modTypes: {user: [], client: [], scheme: [], plugin: []},
              users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
              modUsers: [],
              modUsersMiddleware: [],
              modClients: [],
              modSchemes: [],
              plugins: [],
              invalidCredentialMessage: true
            });
          }
        })
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-edit-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchPlugins()
        .always(() => {
          this.setState({ModModal: {data: {}, callback: false, types: []}, savedRecord: false, savedIndex: -1}, () => {
            $("#editPluginModal").modal("hide");
          });
        });
      })
    } else {
      var plugins = this.state.plugins;
      plugins[this.state.savedIndex] = JSON.parse(this.state.savedRecord);
      this.setState({plugins: plugins, savedRecord: false, savedIndex: -1}, () => {
        $("#editPluginModal").modal("hide");
      });
    }
  }

  confirmDeletePluginMod(result) {
    if (result) {
      apiManager.glewlwydRequest("/mod/plugin/" + encodeURIComponent(this.state.curMod.name), "DELETE")
      .then(() => {
        messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-mod")});
      })
      .fail((err) => {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.error-api-delete-mod")});
        if (err.status !== 401) {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
        } else {
          messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
          this.setState({
            loggedIn: false,
            modTypes: {user: [], client: [], scheme: [], plugin: []},
            users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
            modUsers: [],
            modUsersMiddleware: [],
            modClients: [],
            modSchemes: [],
            plugins: [],
            invalidCredentialMessage: true
          });
        }
      })
      .always(() => {
        this.fetchPlugins()
        .always(() => {
          this.setState({confirmModal: {title: "", message: ""}}, () => {
            $("#confirmModal").modal("hide");
          });
        });
      });
    } else {
      this.setState({confirmModal: {title: "", message: ""}}, () => {
        $("#confirmModal").modal("hide");
      });
    }
  }

  addApiKey() {
    apiManager.glewlwydRequest("/key", "POST")
    .then((result) => {
      var messageModal = this.state.messageModal;
      messageModal.title = i18next.t("admin.api-key-add-title");
      messageModal.label = i18next.t("admin.api-key-add-label");
      messageModal.message = [result.key];
      this.setState({messageModal: messageModal}, () => {
        $("#messageModal").modal({keyboard: false, show: true});
      });
      this.fetchApiKeys();
    })
    .fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
      } else {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    })
  }

  confirmDisableApiKey() {
    apiManager.glewlwydRequest("/key/" + encodeURIComponent(this.state.curApiKey.token_hash), "DELETE")
    .then((key) => {
      messageDispatcher.sendMessage('Notification', {type: "success", message: i18next.t("admin.success-api-delete-api-key")});
    })
    .fail((err) => {
      if (err.status !== 401) {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("error-api-connect")});
      } else {
        messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("admin.requires-admin-scope")});
        this.setState({
          loggedIn: false,
          modTypes: {user: [], client: [], scheme: [], plugin: []},
          users: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          clients: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          scopes: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          apiKeys: {list: [], offset: 0, limit: 20, searchPattern: "", pattern: false},
          modUsers: [],
          modUsersMiddleware: [],
          modClients: [],
          modSchemes: [],
          plugins: [],
          invalidCredentialMessage: true
        });
      }
    })
    .always(() => {
      $("#confirmModal").modal("hide");
      this.fetchApiKeys();
    });
  }

	render() {
    var invalidCredentialMessage;
    if (this.state.invalidCredentialMessage) {
      invalidCredentialMessage = <div className="alert alert-danger" role="alert">{i18next.t("admin.error-credential-message")}</div>
    }
    if (this.state.config) {
      return (
        <div aria-live="polite" aria-atomic="true" className="glwd-container">
          <div className="card center glwd-card" id="userCard" tabIndex="-1" role="dialog">
            <div className="card-header">
              <Navbar active={this.state.curNav} config={this.state.config} loggedIn={this.state.loggedIn} profileList={this.state.profileList}/>
            </div>
            {invalidCredentialMessage}
            <div className="card-body">
              <div id="carouselBody" className="carousel slide" data-ride="carousel">
                <div className="carousel-inner">
                  <div className={"carousel-item" + (this.state.curNav==="users"?" active":"")}>
                    <Users config={this.state.config} users={this.state.users} source={this.state.modUsers} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="clients"?" active":"")}>
                    <Clients config={this.state.config} clients={this.state.clients} source={this.state.modClients} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="scopes"?" active":"")}>
                    <Scopes config={this.state.config} scopes={this.state.scopes} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="users-mod"?" active":"")}>
                    <UserMod mods={this.state.modUsers} types={this.state.modTypes.user} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="clients-mod"?" active":"")}>
                    <ClientMod mods={this.state.modClients} types={this.state.modTypes.client} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="users-middleware-mod"?" active":"")}>
                    <UserMiddlewareMod mods={this.state.modUsersMiddleware} types={this.state.modTypes.user_middleware} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="auth-schemes"?" active":"")}>
                    <SchemeMod mods={this.state.modSchemes} types={this.state.modTypes.scheme} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="plugins"?" active":"")}>
                    <Plugin mods={this.state.plugins} types={this.state.modTypes.plugin} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="api-key"?" active":"")}>
                    <APIKey config={this.state.config} apiKeys={this.state.apiKeys} loggedIn={this.state.loggedIn} />
                  </div>
                  <div className={"carousel-item" + (this.state.curNav==="misc-config"?" active":"")}>
                    <MiscConfig config={this.state.config} miscConfig={this.state.miscConfig} loggedIn={this.state.loggedIn} />
                  </div>
                </div>
              </div>
            </div>
          </div>
          <Notification loggedIn={this.state.loggedIn}/>
          <Confirm title={this.state.confirmModal.title}
                   message={this.state.confirmModal.message}
                   callback={this.state.confirmModal.callback} />
          <Message title={this.state.messageModal.title}
                   label={this.state.messageModal.label}
                   message={this.state.messageModal.message} />
          <EditRecord title={this.state.editModal.title}
                      pattern={this.state.editModal.pattern}
                      source={this.state.editModal.source}
                      defaultSource={this.state.editModal.defaultSource}
                      data={this.state.editModal.data}
                      callback={this.state.editModal.callback}
                      validateCallback={this.state.editModal.validateCallback}
                      add={this.state.editModal.add} />
          <ScopeEdit title={this.state.scopeModal.title}
                     scope={this.state.scopeModal.data}
                     add={this.state.scopeModal.add}
                     modSchemes={this.state.modSchemes}
                     callback={this.state.scopeModal.callback} />
          <ModEdit title={this.state.ModModal.title}
                   role={this.state.ModModal.role}
                   mod={this.state.ModModal.data}
                   add={this.state.ModModal.add}
                   types={this.state.ModModal.types}
                   callback={this.state.ModModal.callback}
                   config={this.state.config}
                   miscConfig={this.state.miscConfig} />
          <PluginEdit title={this.state.PluginModal.title}
                      mod={this.state.PluginModal.data}
                      add={this.state.PluginModal.add}
                      modSchemes={this.state.modSchemes}
                      types={this.state.PluginModal.types}
                      callback={this.state.PluginModal.callback}
                      config={this.state.config}
                      miscConfig={this.state.miscConfig}/>
        </div>
      );
    } else {
      return (
        <div aria-live="polite" aria-atomic="true" className="glwd-container">
          <div className="card center glwd-card" id="userCard" tabIndex="-1" role="dialog">
            <div className="card-header">
              <h4>
                <span className="badge badge-danger">
                  {i18next.t("error-api-connect")}
                </span>
              </h4>
            </div>
          </div>
        </div>
      );
    }
	}
}

export default App;
