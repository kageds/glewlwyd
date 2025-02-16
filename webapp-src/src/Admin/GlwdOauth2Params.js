import React, { Component } from 'react';
import i18next from 'i18next';

import messageDispatcher from '../lib/MessageDispatcher';

class GlwdOauth2Params extends Component {
  constructor(props) {
    super(props);
    
    props.mod.parameters?"":(props.mod.parameters = {});
    props.mod.parameters["jwt-type"]?"":(props.mod.parameters["jwt-type"] = "rsa");
    props.mod.parameters["jwt-key-size"]?"":(props.mod.parameters["jwt-key-size"] = "512");
    props.mod.parameters["key"]?"":(props.mod.parameters["key"] = "");
    props.mod.parameters["cert"]?"":(props.mod.parameters["cert"] = "");
    props.mod.parameters["cert"]?"":(props.mod.parameters["cert"] = "");
    props.mod.parameters["access-token-duration"]?"":(props.mod.parameters["access-token-duration"] = 3600);
    props.mod.parameters["refresh-token-duration"]?"":(props.mod.parameters["refresh-token-duration"] = 1209600);
    props.mod.parameters["code-duration"]?"":(props.mod.parameters["code-duration"] = 600);
    props.mod.parameters["refresh-token-rolling"]!==undefined?"":(props.mod.parameters["refresh-token-rolling"] = true);
    props.mod.parameters["auth-type-code-enabled"]!==undefined?"":(props.mod.parameters["auth-type-code-enabled"] = true);
    props.mod.parameters["auth-type-code-revoke-replayed"]!==undefined?"":(props.mod.parameters["auth-type-code-revoke-replayed"] = false);
    props.mod.parameters["auth-type-implicit-enabled"]!==undefined?"":(props.mod.parameters["auth-type-implicit-enabled"] = true);
    props.mod.parameters["auth-type-password-enabled"]!==undefined?"":(props.mod.parameters["auth-type-password-enabled"] = true);
    props.mod.parameters["auth-type-client-enabled"]!==undefined?"":(props.mod.parameters["auth-type-client-enabled"] = true);
    props.mod.parameters["auth-type-device-enabled"]!==undefined?"":(props.mod.parameters["auth-type-device-enabled"] = false);
    props.mod.parameters["auth-type-refresh-enabled"]!==undefined?"":(props.mod.parameters["auth-type-refresh-enabled"] = true);
    props.mod.parameters["scope"]?"":(props.mod.parameters["scope"] = []);
    props.mod.parameters["additional-parameters"]?"":(props.mod.parameters["additional-parameters"] = []);
    props.mod.parameters["pkce-allowed"]!==undefined?"":(props.mod.parameters["pkce-allowed"] = false);
    props.mod.parameters["pkce-method-plain-allowed"]!==undefined?"":(props.mod.parameters["pkce-method-plain-allowed"] = false);
    props.mod.parameters["introspection-revocation-allowed"]!==undefined?"":(props.mod.parameters["introspection-revocation-allowed"] = false);
    props.mod.parameters["introspection-revocation-auth-scope"]!==undefined?"":(props.mod.parameters["introspection-revocation-auth-scope"] = []);
    props.mod.parameters["introspection-revocation-allow-target-client"]!==undefined?"":(props.mod.parameters["introspection-revocation-allow-target-client"] = true);
    props.mod.parameters["device-authorization-expiration"]!==undefined?"":(props.mod.parameters["device-authorization-expiration"] = 600);
    props.mod.parameters["device-authorization-interval"]!==undefined?"":(props.mod.parameters["device-authorization-interval"] = 5);

    this.state = {
      config: props.config,
      mod: props.mod,
      role: props.role,
      check: props.check,
      errorList: {},
      newScopeOverride: false
    };
    
    if (this.state.check) {
      this.checkParameters();
    }
    
    this.checkParameters = this.checkParameters.bind(this);
    this.changeParam = this.changeParam.bind(this);
    this.changeNumberParam = this.changeNumberParam.bind(this);
    this.toggleParam = this.toggleParam.bind(this);
    this.changeJwtType = this.changeJwtType.bind(this);
    this.setNewScopeOverride = this.setNewScopeOverride.bind(this);
    this.addScopeOverride = this.addScopeOverride.bind(this);
    this.changeScopeOverrideRefreshDuration = this.changeScopeOverrideRefreshDuration.bind(this);
    this.toggleScopeOverrideRolling = this.toggleScopeOverrideRolling.bind(this);
    this.addAdditionalParameter = this.addAdditionalParameter.bind(this);
    this.setAdditionalPropertyUserParameter = this.setAdditionalPropertyUserParameter.bind(this);
    this.setAdditionalPropertyTokenParameter = this.setAdditionalPropertyTokenParameter.bind(this);
    this.deleteAdditionalProperty = this.deleteAdditionalProperty.bind(this);
    this.addScope = this.addScope.bind(this);
    this.deleteScope = this.deleteScope.bind(this);
  }
  
  componentWillReceiveProps(nextProps) {
    
    nextProps.mod.parameters?"":(nextProps.mod.parameters = {});
    nextProps.mod.parameters["jwt-type"]?"":(nextProps.mod.parameters["jwt-type"] = "rsa");
    nextProps.mod.parameters["jwt-key-size"]?"":(nextProps.mod.parameters["jwt-key-size"] = "512");
    nextProps.mod.parameters["key"]?"":(nextProps.mod.parameters["key"] = "");
    nextProps.mod.parameters["cert"]?"":(nextProps.mod.parameters["cert"] = "");
    nextProps.mod.parameters["cert"]?"":(nextProps.mod.parameters["cert"] = "");
    nextProps.mod.parameters["access-token-duration"]?"":(nextProps.mod.parameters["access-token-duration"] = 3600);
    nextProps.mod.parameters["refresh-token-duration"]?"":(nextProps.mod.parameters["refresh-token-duration"] = 1209600);
    nextProps.mod.parameters["code-duration"]?"":(nextProps.mod.parameters["code-duration"] = 600);
    nextProps.mod.parameters["refresh-token-rolling"]!==undefined?"":(nextProps.mod.parameters["refresh-token-rolling"] = true);
    nextProps.mod.parameters["auth-type-code-enabled"]!==undefined?"":(nextProps.mod.parameters["auth-type-code-enabled"] = true);
    nextProps.mod.parameters["auth-type-code-revoke-replayed"]!==undefined?"":(nextProps.mod.parameters["auth-type-code-revoke-replayed"] = false);
    nextProps.mod.parameters["auth-type-implicit-enabled"]!==undefined?"":(nextProps.mod.parameters["auth-type-implicit-enabled"] = true);
    nextProps.mod.parameters["auth-type-password-enabled"]!==undefined?"":(nextProps.mod.parameters["auth-type-password-enabled"] = true);
    nextProps.mod.parameters["auth-type-client-enabled"]!==undefined?"":(nextProps.mod.parameters["auth-type-client-enabled"] = true);
    nextProps.mod.parameters["auth-type-device-enabled"]!==undefined?"":(nextProps.mod.parameters["auth-type-device-enabled"] = false);
    nextProps.mod.parameters["auth-type-refresh-enabled"]!==undefined?"":(nextProps.mod.parameters["auth-type-refresh-enabled"] = true);
    nextProps.mod.parameters["scope"]?"":(nextProps.mod.parameters["scope"] = []);
    nextProps.mod.parameters["additional-parameters"]?"":(nextProps.mod.parameters["additional-parameters"] = []);
    nextProps.mod.parameters["pkce-allowed"]!==undefined?"":(nextProps.mod.parameters["pkce-allowed"] = false);
    nextProps.mod.parameters["pkce-method-plain-allowed"]!==undefined?"":(nextProps.mod.parameters["pkce-method-plain-allowed"] = false);
    nextProps.mod.parameters["introspection-revocation-allowed"]!==undefined?"":(nextProps.mod.parameters["introspection-revocation-allowed"] = false);
    nextProps.mod.parameters["introspection-revocation-auth-scope"]!==undefined?"":(nextProps.mod.parameters["introspection-revocation-auth-scope"] = []);
    nextProps.mod.parameters["introspection-revocation-allow-target-client"]!==undefined?"":(nextProps.mod.parameters["introspection-revocation-allow-target-client"] = true);
    nextProps.mod.parameters["device-authorization-expiration"]!==undefined?"":(nextProps.mod.parameters["device-authorization-expiration"] = 600);
    nextProps.mod.parameters["device-authorization-interval"]!==undefined?"":(nextProps.mod.parameters["device-authorization-interval"] = 5);

    this.setState({
      config: nextProps.config,
      mod: nextProps.mod,
      role: nextProps.role,
      check: nextProps.check
    }, () => {
      if (this.state.check) {
        this.checkParameters();
      }
    });
  }
  
  changeParam(e, param) {
    var mod = this.state.mod;
    mod.parameters[param] = e.target.value;
    this.setState({mod: mod});
  }
  
  changeNumberParam(e, param) {
    var mod = this.state.mod;
    mod.parameters[param] = parseInt(e.target.value);
    if (!isNaN(mod.parameters[param])) {
      this.setState({mod: mod});
    }
  }
  
  toggleParam(e, param) {
    var mod = this.state.mod;
    mod.parameters[param] = !mod.parameters[param];
    this.setState({mod: mod});
  }
  
  changeJwtType(e, type) {
    var mod = this.state.mod;
    mod.parameters["jwt-type"] = type;
    this.setState({mod: mod});
  }
  
  changeJwtKeySize(e, size) {
    var mod = this.state.mod;
    mod.parameters["jwt-key-size"] = size;
    this.setState({mod: mod});
  }
  
  uploadFile(e, name) {
    var mod = this.state.mod;
    var file = e.target.files[0];
    var fr = new FileReader();
    fr.onload = (ev2) => {
      mod.parameters[name] = ev2.target.result;
      this.setState({mod: mod});
    };
    fr.readAsText(file);
  }
  
  setNewScopeOverride(e, scope) {
    this.setState({newScopeOverride: scope});
  }
  
  addScopeOverride() {
    if (this.state.newScopeOverride) {
      var mod = this.state.mod;
      mod.parameters["scope"].push({
        name: this.state.newScopeOverride, 
        "refresh-token-rolling": this.state.mod.parameters["refresh-token-rolling"], 
        "refresh-token-duration": 0
      });
      this.setState({mod: mod, newScopeOverride: false});
    }
  }
  
  changeScopeOverrideRefreshDuration(e, scope) {
    var mod = this.state.mod;
    mod.parameters["scope"].forEach((curScope) => {
      if (curScope.name === scope.name) {
        curScope["refresh-token-duration"] = parseInt(e.target.value);
      }
    });
    this.setState({mod: mod});
  }
  
  toggleScopeOverrideRolling(e, scope, value) {
    var mod = this.state.mod;
    mod.parameters["scope"].forEach((curScope) => {
      if (curScope.name === scope) {
        if (value === undefined) {
          delete (curScope["refresh-token-rolling"]);
        } else {
          curScope["refresh-token-rolling"] = value;
        }
      }
    });
    this.setState({mod: mod});
  }
  
  deleteScopeOverride(e, scope) {
    var mod = this.state.mod;
    mod.parameters["scope"].forEach((curScope, index) => {
      if (curScope.name === scope) {
        mod.parameters["scope"].splice(index, 1);
      }
    });
    this.setState({mod: mod});
  }
  
  addAdditionalParameter() {
    var mod = this.state.mod;
    mod.parameters["additional-parameters"].push({
      "user-parameter": "", 
      "token-parameter": "", 
      "token-changed": false
    });
    this.setState({mod: mod, newScopeOverride: false});
  }
  
  setAdditionalPropertyUserParameter(e, index) {
    var mod = this.state.mod;
    if (mod.parameters["additional-parameters"][index]) {
      mod.parameters["additional-parameters"][index]["user-parameter"] = e.target.value;
      if (!mod.parameters["additional-parameters"][index]["token-changed"]) {
        mod.parameters["additional-parameters"][index]["token-parameter"] = e.target.value;
      }
    }
    this.setState({mod: mod, newScopeOverride: false});
  }
  
  setAdditionalPropertyTokenParameter(e, index) {
    var mod = this.state.mod;
    if (mod.parameters["additional-parameters"][index]) {
      mod.parameters["additional-parameters"][index]["token-parameter"] = e.target.value;
      mod.parameters["additional-parameters"][index]["token-changed"] = true;
    }
    this.setState({mod: mod, newScopeOverride: false});
  }
  
  deleteAdditionalProperty(e, index) {
    var mod = this.state.mod;
    if (mod.parameters["additional-parameters"][index]) {
      mod.parameters["additional-parameters"].splice(index, 1);
    }
    this.setState({mod: mod, newScopeOverride: false});
  }
  
  addScope(e, scope) {
    e.preventDefault();
    var mod = this.state.mod;
    mod.parameters["introspection-revocation-auth-scope"].push(scope);
    this.setState({mod: mod});
  }

  deleteScope(e, index) {
    e.preventDefault();
    var mod = this.state.mod;
    mod.parameters["introspection-revocation-auth-scope"].splice(index, 1);
    this.setState({mod: mod});
  }
  
  checkParameters() {
    var errorList = {}, hasError = false;
    if (!this.state.mod.parameters["key"]) {
      hasError = true;
      errorList["key"] = "admin.mod-glwd-key-error";
      errorList["signature"] = true;
    }
    if (this.state.mod.parameters["jwt-type"] !== "sha" && !this.state.mod.parameters["cert"]) {
      hasError = true;
      errorList["cert"] = "admin.mod-glwd-cert-error";
      errorList["signature"] = true;
    }
    if (!this.state.mod.parameters["access-token-duration"]) {
      hasError = true;
      errorList["access-token-duration"] = "admin.mod-glwd-access-token-duration-error";
      errorList["token"] = true;
    }
    if (!this.state.mod.parameters["refresh-token-duration"]) {
      hasError = true;
      errorList["refresh-token-duration"] = "admin.mod-glwd-refresh-token-duration-error";
      errorList["token"] = true;
    }
    if (!this.state.mod.parameters["code-duration"]) {
      hasError = true;
      errorList["code-duration"] = "admin.mod-glwd-code-duration-error";
      errorList["token"] = true;
    }
    this.state.mod.parameters["additional-parameters"].forEach((addParam, index) => {
      if (!addParam["user-parameter"]) {
        hasError = true;
        if (!errorList["additional-parameters"]) {
          errorList["additional-parameters"] = [];
        }
        if (!errorList["additional-parameters"][index]) {
          errorList["additional-parameters"][index] = {};
        }
        errorList["additional-parameters"][index]["user"] = "admin.mod-glwd-additional-parameter-user-parameter-error";
      }
      if (!addParam["token-parameter"]) {
        hasError = true;
        if (!errorList["additional-parameters"]) {
          errorList["additional-parameters"] = [];
        }
        if (!errorList["additional-parameters"][index]) {
          errorList["additional-parameters"][index] = {};
        }
        errorList["additional-parameters"][index]["token"] = "admin.mod-glwd-additional-parameter-token-parameter-error";
      } else if (addParam["token-parameter"] === "username" || 
                 addParam["token-parameter"] === "salt" || 
                 addParam["token-parameter"] === "type" ||
                 addParam["token-parameter"] === "iat" ||
                 addParam["token-parameter"] === "expires_in" ||
                 addParam["token-parameter"] === "scope") {
        hasError = true;
        if (!errorList["additional-parameters"]) {
          errorList["additional-parameters"] = [];
        }
        if (!errorList["additional-parameters"][index]) {
          errorList["additional-parameters"][index] = {};
        }
        errorList["additional-parameters"][index]["token"] = "admin.mod-glwd-additional-parameter-token-parameter-invalid-error";
      }
    });
    if (this.state.mod.parameters["introspection-revocation-allowed"] && !this.state.mod.parameters["introspection-revocation-allow-target-client"] && !this.state.mod.parameters["introspection-revocation-auth-scope"].length) {
      hasError = true;
      errorList["introspection-revocation"] = "admin.mod-glwd-introspection-revocation-error";
      errorList["token"] = true;
    }
    if (!hasError) {
      this.setState({errorList: {}}, () => {
        messageDispatcher.sendMessage('ModPlugin', {type: "modValid"});
      });
    } else {
      this.setState({errorList: errorList}, () => {
        messageDispatcher.sendMessage('ModPlugin', {type: "modInvalid"});
      });
    }
  }
  
  render() {
    var keyJsx, certJsx, scopeOverrideList = [], scopeList = [], additionalParametersList = [];
    var baseApiUrl = document.location.href.split('?')[0].split('#')[0];
    if (baseApiUrl.endsWith(this.state.config.AdminUrl)) {
      baseApiUrl = baseApiUrl.substring(0, baseApiUrl.length-this.state.config.AdminUrl.length-1) + "/";
    }
    baseApiUrl += this.state.config.api_prefix + "/" + (this.state.mod.name||"");
    var urlAuth = baseApiUrl + "/auth", urlToken = baseApiUrl + "/token", urlProfile = baseApiUrl + "/profile";
    if (this.state.mod.parameters["jwt-type"] === "sha") {
      keyJsx =
        <div className="form-group">
          <div className="input-group mb-3">
            <div className="input-group-prepend">
              <label className="input-group-text" htmlFor="mod-glwd-key">{i18next.t("admin.mod-glwd-key")}</label>
            </div>
            <input type="password" className={this.state.errorList["key"]?"form-control is-invalid":"form-control"} id="mod-glwd-key" onChange={(e) => this.changeParam(e, "key")} value={this.state.mod.parameters["key"]} placeholder={i18next.t("admin.mod-glwd-key-ph")} />
          </div>
          {this.state.errorList["key"]?<span className="error-input">{this.state.errorList["key"]}</span>:""}
        </div>;
    } else {
      keyJsx =
        <div className="form-group">
          <div className="input-group mb-3">
            <div className="input-group-prepend">
              <label className="input-group-text" htmlFor="mod-glwd-key">{i18next.t("admin.mod-glwd-key")}</label>
            </div>
            <div className="custom-file">
              <input type="file" className={this.state.errorList["key"]?"custom-file-input is-invalid":"custom-file-input"} onChange={(e) => this.uploadFile(e, "key")} />
              <label className="custom-file-label" htmlFor="inputGroupFile01">{i18next.t("admin.choose-file")}</label>
            </div>
          </div>
          {this.state.mod.parameters["key"]?<div className="alert alert-primary">{this.state.mod.parameters["key"].substring(0, 40)}</div>:""}
          {this.state.errorList["key"]?<span className="error-input">{this.state.errorList["key"]}</span>:""}
        </div>;
      certJsx =
        <div className="form-group">
          <div className="input-group mb-3">
            <div className="input-group-prepend">
              <label className="input-group-text" htmlFor="mod-glwd-cert">{i18next.t("admin.mod-glwd-cert")}</label>
            </div>
            <div className="custom-file">
              <input type="file" className={this.state.errorList["key"]?"custom-file-input is-invalid":"custom-file-input"} onChange={(e) => this.uploadFile(e, "cert")} />
              <label className="custom-file-label" htmlFor="inputGroupFile01">{i18next.t("admin.choose-file")}</label>
            </div>
          </div>
          {this.state.mod.parameters["cert"]?<div className="alert alert-primary">{this.state.mod.parameters["cert"].substring(0, 40)}</div>:""}
          {this.state.errorList["cert"]?<span className="error-input">{this.state.errorList["cert"]}</span>:""}
        </div>;
    }
    this.state.config.pattern.user.forEach((pattern) => {
      if (pattern.name === "scope") {
        pattern.listElements.forEach((scope, index) => {
          var found = 0;
          this.state.mod.parameters["scope"].forEach((curScope) => {
            if (curScope.name === scope) {
              found = 1;
            }
          });
          if (!found) {
            scopeList.push(<a key={index} className="dropdown-item" href="#" onClick={(e) => this.setNewScopeOverride(e, scope)}>{scope}</a>);
          }
        });
      }
    });
    var scopeJsx = 
      <div className="dropdown">
        <button className="btn btn-secondary dropdown-toggle" type="button" id="mod-glwd-scope-override-scope" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          {this.state.newScopeOverride||i18next.t("admin.mod-glwd-scope-override-scope")}
        </button>
        <div className="dropdown-menu" aria-labelledby="mod-glwd-scope-override-scope">
          {scopeList}
        </div>
      </div>;
    this.state.mod.parameters["scope"].forEach((scope, index) => {
      scopeOverrideList.push(
      <div key={index}>
        <hr/>
        <h4>{scope.name}</h4>
        <div className="form-group">
          <div className="input-group mb-3">
            <div className="input-group-prepend">
              <label className="input-group-text" htmlFor={"mod-glwd-scope-override-refresh-duration-"+scope.name}>{i18next.t("admin.mod-glwd-scope-override-refresh-duration")}</label>
            </div>
            <input type="number" min="0" step="1" className="form-control" id={"mod-glwd-scope-override-refresh-duration-"+scope.name} onChange={(e) => this.changeScopeOverrideRefreshDuration(e, scope)} value={scope["refresh-token-duration"]} placeholder={i18next.t("admin.mod-glwd-scope-override-refresh-duration-ph")} />
          </div>
        </div>
        <div className="form-group">
          <div className="input-group mb-3">
            <div className="input-group-prepend">
              <label className="input-group-text" htmlFor={"mod-glwd-scope-override-refresh-rolling-"+scope.name}>{i18next.t("admin.mod-scope-override-refresh-rolling")}</label>
            </div>
            <div className="dropdown">
              <button className="btn btn-secondary dropdown-toggle" type="button" id={"mod-glwd-scope-override-refresh-rolling-"+scope.name} data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                {i18next.t("admin.glwd-scope-override-refresh-rolling-value-" + scope["refresh-token-rolling"])}
              </button>
              <div className="dropdown-menu" aria-labelledby="mod-glwd-jwt-type">
                <a className={"dropdown-item"+(scope["refresh-token-rolling"]===undefined?" active":"")} href="#" onClick={(e) => this.toggleScopeOverrideRolling(e, scope.name, undefined)}>{i18next.t("admin.glwd-scope-override-refresh-rolling-value-undefined")}</a>
                <a className={"dropdown-item"+(scope["refresh-token-rolling"]===true?" active":"")} href="#" onClick={(e) => this.toggleScopeOverrideRolling(e, scope.name, true)}>{i18next.t("admin.glwd-scope-override-refresh-rolling-value-true")}</a>
                <a className={"dropdown-item"+(scope["refresh-token-rolling"]===false?" active":"")} href="#" onClick={(e) => this.toggleScopeOverrideRolling(e, scope.name, false)}>{i18next.t("admin.glwd-scope-override-refresh-rolling-value-false")}</a>
              </div>
            </div>
          </div>
        </div>
        <button type="button" className="btn btn-secondary" onClick={(e) => this.deleteScopeOverride(e, scope.name)} title={i18next.t("admin.mod-scope-override-delete")}>
          <i className="fas fa-trash"></i>
        </button>
      </div>
      );
    });
    this.state.mod.parameters["additional-parameters"].forEach((parameter, index) => {
      var hasUserError = this.state.errorList["additional-parameters"] && this.state.errorList["additional-parameters"][index] && this.state.errorList["additional-parameters"][index]["user"];
      var hasTokenError = this.state.errorList["additional-parameters"] && this.state.errorList["additional-parameters"][index] && this.state.errorList["additional-parameters"][index]["token"];
      additionalParametersList.push(
      <div key={index}>
        <hr/>
        <h4>{parameter["user-parameter"]||i18next.t("admin.mod-additional-parameter-new")}</h4>
        <div className="form-group">
          <div className="input-group mb-3">
            <div className="input-group-prepend">
              <label className="input-group-text" htmlFor={"mod-glwd-additional-parameter-user-parameter-"+parameter["user-parameter"]}>{i18next.t("admin.mod-glwd-additional-parameter-user-parameter")}</label>
            </div>
            <input type="text" className={hasUserError?"form-control is-invalid":"form-control"} id={"mod-glwd-additional-parameter-user-parameter-"+parameter["user-parameter"]} onChange={(e) => this.setAdditionalPropertyUserParameter(e, index)} value={parameter["user-parameter"]} placeholder={i18next.t("admin.mod-glwd-additional-parameter-user-parameter-ph")} />
            {hasUserError?<span className="error-input">{this.state.errorList["additional-parameters"][index]["user"]}</span>:""}
          </div>
        </div>
        <div className="form-group">
          <div className="input-group mb-3">
            <div className="input-group-prepend">
              <label className="input-group-text" htmlFor={"mod-glwd-additional-parameter-token-parameter-"+parameter["token-parameter"]}>{i18next.t("admin.mod-glwd-additional-parameter-token-parameter")}</label>
            </div>
            <input type="text" className={hasTokenError?"form-control is-invalid":"form-control"} id={"mod-glwd-additional-parameter-token-parameter-"+parameter["token-parameter"]} onChange={(e) => this.setAdditionalPropertyTokenParameter(e, index)} value={parameter["token-parameter"]} placeholder={i18next.t("admin.mod-glwd-additional-parameter-token-parameter-ph")} />
          </div>
          {hasTokenError?<span className="error-input">{this.state.errorList["additional-parameters"][index]["token"]}</span>:""}
        </div>
        <button type="button" className="btn btn-secondary" onClick={(e) => this.deleteAdditionalProperty(e, index)} title={i18next.t("admin.mod-additional-parameter-token-delete")}>
          <i className="fas fa-trash"></i>
        </button>
      </div>
      );
    });
    var scopeList = [], defaultScopeList = [];
    this.state.config.pattern.user.forEach((pattern) => {
      if (pattern.name === "scope") {
        pattern.listElements.forEach((scope, index) => {
          scopeList.push(<a key={index} className="dropdown-item" href="#" onClick={(e) => this.addScope(e, scope)} disabled={!this.state.mod.parameters["introspection-revocation-allowed"]}>{scope}</a>);
        })
      }
    });
    this.state.mod.parameters["introspection-revocation-auth-scope"].forEach((scope, index) => {
      if (this.state.mod.parameters["introspection-revocation-allowed"]) {
        defaultScopeList.push(
          <a className="btn-icon-right" href="#" onClick={(e) => this.deleteScope(e, index)} key={index} >
            <span className="badge badge-primary">{scope}
              <span className="badge badge-light btn-icon-right">
                <i className="fas fa-times"></i>
              </span>
            </span>
          </a>
        );
      } else {
        defaultScopeList.push(<span key={index} className="badge badge-primary btn-icon-right">{scope}</span>);
      }
    });
    var scopeIntrospectJsx = 
    <div className="dropdown">
      <button className="btn btn-secondary dropdown-toggle" type="button" id="mod-register-scope" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false" disabled={!this.state.mod.parameters["introspection-revocation-allowed"]}>{i18next.t("admin.mod-glwd-introspection-revocation-scope")}</button>
      <div className="dropdown-menu" aria-labelledby="mod-register-scope">
        {scopeList}
      </div>
      <div>
        {defaultScopeList}
      </div>
    </div>;
    return (
      <div>
        <div className="form-group">
          <div>
            <div>
              <span className="input-group-text" >{i18next.t("admin.mod-glwd-url-auth")}</span>
            </div>
            <code>
              {urlAuth}
            </code>
          </div>
        </div>
        <div className="form-group">
          <div>
            <div>
              <span className="input-group-text" >{i18next.t("admin.mod-glwd-url-token")}</span>
            </div>
            <code>
              {urlToken}
            </code>
          </div>
        </div>
        <div className="form-group">
          <div>
            <div>
              <span className="input-group-text" >{i18next.t("admin.mod-glwd-url-profile")}</span>
            </div>
            <code>
              {urlProfile}
            </code>
          </div>
        </div>
        <hr/>
        <div className="accordion" id="accordionSignature">
          <div className="card">
            <div className="card-header" id="addParamCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseSignature" aria-expanded="true" aria-controls="collapseSignature">
                  {this.state.errorList["signature"]?<span className="error-input btn-icon"><i className="fas fa-exclamation-circle"></i></span>:""}
                  {i18next.t("admin.mod-glwd-sign-title")}
                </button>
              </h2>
            </div>
            <div id="collapseSignature" className="collapse" aria-labelledby="addParamCard" data-parent="#accordionSignature">
              <div className="card-body">
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-glwd-jwt-type">{i18next.t("admin.mod-glwd-jwt-type")}</label>
                    </div>
                    <div className="dropdown">
                      <button className="btn btn-secondary dropdown-toggle" type="button" id="mod-glwd-jwt-type" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                        {i18next.t("admin.mod-glwd-jwt-type-" + this.state.mod.parameters["jwt-type"])}
                      </button>
                      <div className="dropdown-menu" aria-labelledby="mod-glwd-jwt-type">
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-type"]==="rsa"?" active":"")} href="#" onClick={(e) => this.changeJwtType(e, 'rsa')}>{i18next.t("admin.mod-glwd-jwt-type-rsa")}</a>
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-type"]==="ecdsa"?" active":"")} href="#" onClick={(e) => this.changeJwtType(e, 'ecdsa')}>{i18next.t("admin.mod-glwd-jwt-type-ecdsa")}</a>
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-type"]==="sha"?" active":"")} href="#" onClick={(e) => this.changeJwtType(e, 'sha')}>{i18next.t("admin.mod-glwd-jwt-type-sha")}</a>
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-type"]==="rsa-pss"?" active":"")} href="#" onClick={(e) => this.changeJwtType(e, 'rsa-pss')}>{i18next.t("admin.mod-glwd-jwt-type-rsa-pss")}</a>
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-type"]==="eddsa"?" active":"")} href="#" onClick={(e) => this.changeJwtType(e, 'eddsa')}>{i18next.t("admin.mod-glwd-jwt-type-eddsa")}</a>
                      </div>
                    </div>
                  </div>
                  {this.state.errorList["jwt-type"]?<span className="error-input">{this.state.errorList["jwt-type"]}</span>:""}
                </div>
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-glwd-jwt-key-size">{i18next.t("admin.mod-glwd-jwt-key-size")}</label>
                    </div>
                    <div className="dropdown">
                      <button className="btn btn-secondary dropdown-toggle" type="button" id="mod-glwd-jwt-key-size" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                        {i18next.t("admin.mod-glwd-jwt-key-size-" + this.state.mod.parameters["jwt-key-size"])}
                      </button>
                      <div className="dropdown-menu" aria-labelledby="mod-glwd-jwt-key-size">
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-key-size"]==="256"?" active":"")} href="#" onClick={(e) => this.changeJwtKeySize(e, '256')}>{i18next.t("admin.mod-glwd-jwt-key-size-256")}</a>
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-key-size"]==="384"?" active":"")} href="#" onClick={(e) => this.changeJwtKeySize(e, '384')}>{i18next.t("admin.mod-glwd-jwt-key-size-384")}</a>
                        <a className={"dropdown-item"+(this.state.mod.parameters["jwt-key-size"]==="512"?" active":"")} href="#" onClick={(e) => this.changeJwtKeySize(e, '512')}>{i18next.t("admin.mod-glwd-jwt-key-size-512")}</a>
                      </div>
                    </div>
                  </div>
                  {this.state.errorList["jwt-key-size"]?<span className="error-input">{this.state.errorList["jwt-key-size"]}</span>:""}
                </div>
                {keyJsx}
                {certJsx}
              </div>
            </div>
          </div>
        </div>
        <div className="accordion" id="accordionToken">
          <div className="card">
            <div className="card-header" id="addParamCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseToken" aria-expanded="true" aria-controls="collapseToken">
                  {this.state.errorList["token"]?<span className="error-input btn-icon"><i className="fas fa-exclamation-circle"></i></span>:""}
                  {i18next.t("admin.mod-glwd-token-title")}
                </button>
              </h2>
            </div>
            <div id="collapseToken" className="collapse" aria-labelledby="addParamCard" data-parent="#accordionToken">
              <div className="card-body">
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-glwd-access-token-duration">{i18next.t("admin.mod-glwd-access-token-duration")}</label>
                    </div>
                    <input type="number" min="0" step="1" className={this.state.errorList["access-token-duration"]?"form-control is-invalid":"form-control"} id="mod-glwd-access-token-duration" onChange={(e) => this.changeNumberParam(e, "access-token-duration")} value={this.state.mod.parameters["access-token-duration"]} placeholder={i18next.t("admin.mod-glwd-access-token-duration-ph")} />
                  </div>
                  {this.state.errorList["access-token-duration"]?<span className="error-input">{this.state.errorList["access-token-duration"]}</span>:""}
                </div>
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-glwd-refresh-token-duration">{i18next.t("admin.mod-glwd-refresh-token-duration")}</label>
                    </div>
                    <input type="number" min="0" step="1" className={this.state.errorList["refresh-token-duration"]?"form-control is-invalid":"form-control"} id="mod-glwd-refresh-token-duration" onChange={(e) => this.changeNumberParam(e, "refresh-token-duration")} value={this.state.mod.parameters["refresh-token-duration"]} placeholder={i18next.t("admin.mod-glwd-refresh-token-duration-ph")} />
                  </div>
                  {this.state.errorList["refresh-token-duration"]?<span className="error-input">{this.state.errorList["refresh-token-duration"]}</span>:""}
                </div>
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-glwd-code-duration">{i18next.t("admin.mod-glwd-code-duration")}</label>
                    </div>
                    <input type="number" min="0" step="1" className={this.state.errorList["code-duration"]?"form-control is-invalid":"form-control"} id="mod-glwd-code-duration" onChange={(e) => this.changeNumberParam(e, "code-duration")} value={this.state.mod.parameters["code-duration"]} placeholder={i18next.t("admin.mod-glwd-code-duration-ph")} />
                  </div>
                  {this.state.errorList["code-duration"]?<span className="error-input">{this.state.errorList["code-duration"]}</span>:""}
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-refresh-token-rolling" onChange={(e) => this.toggleParam(e, "refresh-token-rolling")} checked={this.state.mod.parameters["refresh-token-rolling"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-refresh-token-rolling">{i18next.t("admin.mod-glwd-refresh-token-rolling")}</label>
                </div>
              </div>
            </div>
          </div>
        </div>
        <div className="accordion" id="accordionAuthType">
          <div className="card">
            <div className="card-header" id="addParamCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseAuthType" aria-expanded="true" aria-controls="collapseAuthType">
                  {this.state.errorList["auth-type"]?<span className="error-input btn-icon"><i className="fas fa-exclamation-circle"></i></span>:""}
                  {i18next.t("admin.mod-glwd-auth-type-title")}
                </button>
              </h2>
            </div>
            <div id="collapseAuthType" className="collapse" aria-labelledby="addParamCard" data-parent="#accordionAuthType">
              <div className="card-body">
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-auth-type-code-enabled" onChange={(e) => this.toggleParam(e, "auth-type-code-enabled")} checked={this.state.mod.parameters["auth-type-code-enabled"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-auth-type-code-enabled">{i18next.t("admin.mod-glwd-auth-type-code-enabled")}</label>
                </div>
                <div className="form-group row">
                  <div className="col-sm-1">
                  </div>
                  <div className="col-sm-11">
                    <div className="form-check">
                      <input type="checkbox" className="form-check-input" id="mod-glwd-auth-type-code-revoke-replayed" onChange={(e) => this.toggleParam(e, "auth-type-code-revoke-replayed")} disabled={!this.state.mod.parameters["auth-type-code-enabled"]} checked={this.state.mod.parameters["auth-type-code-revoke-replayed"]} />
                      <label className="form-check-label" htmlFor="mod-glwd-auth-type-code-revoke-replayed">{i18next.t("admin.mod-glwd-auth-type-code-revoke-replayed")}</label>
                    </div>
                  </div>
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-auth-type-implicit-enabled" onChange={(e) => this.toggleParam(e, "auth-type-implicit-enabled")} checked={this.state.mod.parameters["auth-type-implicit-enabled"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-auth-type-implicit-enabled">{i18next.t("admin.mod-glwd-auth-type-implicit-enabled")}</label>
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-auth-type-password-enabled" onChange={(e) => this.toggleParam(e, "auth-type-password-enabled")} checked={this.state.mod.parameters["auth-type-password-enabled"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-auth-type-password-enabled">{i18next.t("admin.mod-glwd-auth-type-password-enabled")}</label>
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-auth-type-client-enabled" onChange={(e) => this.toggleParam(e, "auth-type-client-enabled")} checked={this.state.mod.parameters["auth-type-client-enabled"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-auth-type-client-enabled">{i18next.t("admin.mod-glwd-auth-type-client-enabled")}</label>
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-auth-type-device-enabled" onChange={(e) => this.toggleParam(e, "auth-type-device-enabled")} checked={this.state.mod.parameters["auth-type-device-enabled"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-auth-type-device-enabled">{i18next.t("admin.mod-glwd-auth-type-device-enabled")}</label>
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-auth-type-refresh-enabled" onChange={(e) => this.toggleParam(e, "auth-type-refresh-enabled")} checked={this.state.mod.parameters["auth-type-refresh-enabled"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-auth-type-refresh-enabled">{i18next.t("admin.mod-glwd-auth-type-refresh-enabled")}</label>
                </div>
              </div>
            </div>
          </div>
        </div>
        <div className="accordion" id="accordionScope">
          <div className="card">
            <div className="card-header" id="dataFormatCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseScope" aria-expanded="true" aria-controls="collapseScope">
                  {this.state.errorList["scope-override"]?<span className="error-input btn-icon"><i className="fas fa-exclamation-circle"></i></span>:""}
                  {i18next.t("admin.mod-glwd-scope-override")}
                </button>
              </h2>
            </div>
            <div id="collapseScope" className="collapse" aria-labelledby="dataFormatCard" data-parent="#accordionScope">
              <div className="card-body">
                <p>{i18next.t("admin.mod-glwd-scope-override-message")}</p>
                <div className="btn-group" role="group">
                  <div className="btn-group" role="group">
                    {scopeJsx}
                  </div>
                  <button type="button" className="btn btn-secondary" onClick={this.addScopeOverride} title={i18next.t("admin.mod-glwd-scope-add")}>
                    <i className="fas fa-plus"></i>
                  </button>
                </div>
                {scopeOverrideList}
              </div>
            </div>
          </div>
        </div>
        <div className="accordion" id="accordionAddParam">
          <div className="card">
            <div className="card-header" id="addParamCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseAddParam" aria-expanded="true" aria-controls="collapseAddParam">
                  {this.state.errorList["additional-parameters"]?<span className="error-input btn-icon"><i className="fas fa-exclamation-circle"></i></span>:""}
                  {i18next.t("admin.mod-glwd-additional-parameter")}
                </button>
              </h2>
            </div>
            <div id="collapseAddParam" className="collapse" aria-labelledby="addParamCard" data-parent="#accordionAddParam">
              <div className="card-body">
                <p>{i18next.t("admin.mod-glwd-additional-parameter-message")}</p>
                <div className="btn-group" role="group">
                  <button type="button" className="btn btn-secondary" onClick={this.addAdditionalParameter} title={i18next.t("admin.mod-glwd-additional-parameter-add")}>
                    <i className="fas fa-plus"></i>
                  </button>
                </div>
                {additionalParametersList}
              </div>
            </div>
          </div>
        </div>
        <div className="accordion" id="accordionPkce">
          <div className="card">
            <div className="card-header" id="addParamCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapsePkce" aria-expanded="true" aria-controls="collapsePkce">
                  {this.state.errorList["pkce"]?<span className="error-input btn-icon"><i className="fas fa-exclamation-circle"></i></span>:""}
                  {i18next.t("admin.mod-glwd-pkce-title")}
                </button>
              </h2>
            </div>
            <div id="collapsePkce" className="collapse" aria-labelledby="addParamCard" data-parent="#accordionPkce">
              <div className="card-body">
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-pkce-allowed" onChange={(e) => this.toggleParam(e, "pkce-allowed")} checked={this.state.mod.parameters["pkce-allowed"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-pkce-allowed">{i18next.t("admin.mod-glwd-pkce-allowed")}</label>
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-pkce-method-plain-allowed" onChange={(e) => this.toggleParam(e, "pkce-method-plain-allowed")} checked={this.state.mod.parameters["pkce-method-plain-allowed"]} disabled={!this.state.mod.parameters["pkce-allowed"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-pkce-method-plain-allowed">{i18next.t("admin.mod-glwd-pkce-method-plain-allowed")}</label>
                </div>
              </div>
            </div>
          </div>
        </div>
        <div className="accordion" id="accordionIntrospect">
          <div className="card">
            <div className="card-header" id="addParamCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseIntrospect" aria-expanded="true" aria-controls="collapseIntrospect">
                  {this.state.errorList["introspection-revocation"]?<span className="error-input btn-icon"><i className="fas fa-exclamation-circle"></i></span>:""}
                  {i18next.t("admin.mod-glwd-introspection-revocation-title")}
                </button>
              </h2>
            </div>
            <div id="collapseIntrospect" className="collapse" aria-labelledby="addParamCard" data-parent="#accordionIntrospect">
              <div className="card-body">
                {this.state.errorList["introspection-revocation"]?<span className="error-input">{i18next.t(this.state.errorList["introspection-revocation"])}</span>:""}
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-introspection-revocation-allowed" onChange={(e) => this.toggleParam(e, "introspection-revocation-allowed")} checked={this.state.mod.parameters["introspection-revocation-allowed"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-introspection-revocation-allowed">{i18next.t("admin.mod-glwd-introspection-revocation-allowed")}</label>
                </div>
                <div className="form-group form-check">
                  <input type="checkbox" className="form-check-input" id="mod-glwd-introspection-revocation-allow-target-client" onChange={(e) => this.toggleParam(e, "introspection-revocation-allow-target-client")} checked={this.state.mod.parameters["introspection-revocation-allow-target-client"]} disabled={!this.state.mod.parameters["introspection-revocation-allowed"]} />
                  <label className="form-check-label" htmlFor="mod-glwd-introspection-revocation-allow-target-client">{i18next.t("admin.mod-glwd-introspection-revocation-allow-target-client")}</label>
                </div>
                <hr/>
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-default-scope">{i18next.t("admin.mod-glwd-introspection-revocation-scope-required")}</label>
                    </div>
                    {scopeIntrospectJsx}
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
        <div className="accordion" id="accordionDeviceAuthorization">
          <div className="card">
            <div className="card-header" id="addParamCard">
              <h2 className="mb-0">
                <button className="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseDeviceAuthorization" aria-expanded="true" aria-controls="collapseDeviceAuthorization">
                  {i18next.t("admin.mod-glwd-device-authorization-title")}
                </button>
              </h2>
            </div>
            <div id="collapseDeviceAuthorization" className="collapse" aria-labelledby="addParamCard" data-parent="#accordionDeviceAuthorization">
              <div className="card-body">
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-glwd-device-authorization-expiration">{i18next.t("admin.mod-glwd-device-authorization-expiration")}</label>
                    </div>
                    <input type="number" min="1" step="1" className="form-control" id="mod-glwd-device-authorization-expiration" onChange={(e) => this.changeNumberParam(e, "device-authorization-expiration")} value={this.state.mod.parameters["device-authorization-expiration"]} placeholder={i18next.t("admin.mod-glwd-device-authorization-expiration-ph")} disabled={!this.state.mod.parameters["auth-type-device-enabled"]} />
                  </div>
                </div>
                <div className="form-group">
                  <div className="input-group mb-3">
                    <div className="input-group-prepend">
                      <label className="input-group-text" htmlFor="mod-glwd-device-authorization-interval">{i18next.t("admin.mod-glwd-device-authorization-interval")}</label>
                    </div>
                    <input type="number" min="1" step="1" className="form-control" id="mod-glwd-device-authorization-interval" onChange={(e) => this.changeNumberParam(e, "device-authorization-interval")} value={this.state.mod.parameters["device-authorization-interval"]} placeholder={i18next.t("admin.mod-glwd-device-authorization-interval-ph")} disabled={!this.state.mod.parameters["auth-type-device-enabled"]} />
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

export default GlwdOauth2Params;
