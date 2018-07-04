/*---------------------------------------------------------
 * Copyright (C) Microsoft Corporation. All rights reserved.
 *--------------------------------------------------------*/

'use strict';

import * as vscode from 'vscode';
import {WorkspaceFolder, DebugConfiguration, ProviderResult, CancellationToken} from 'vscode';
// import {MockDebugSession} from './mockDebug';
import * as Net from 'net';


export function activate(context: vscode.ExtensionContext) {
  context.subscriptions.push(vscode.commands.registerCommand('extension.mock-debug.getProgramName', config => {
    return vscode.window.showInputBox(
        {placeHolder: 'Please enter the name of a markdown file in the workspace folder', value: 'readme.md'});
  }));

  // register a configuration provider for 'mock' debug type
  const provider = new MockConfigurationProvider();
  context.subscriptions.push(vscode.debug.registerDebugConfigurationProvider('mock', provider));
  context.subscriptions.push(provider);
}

export function deactivate() {
  // nothing to do
}

class MockConfigurationProvider implements vscode.DebugConfigurationProvider {
  private _server?: Net.Server;

  /**
   * Massage a debug configuration just before a debug session is being
   * launched,vscode. e.g. add all missing attributes to the debug
   * configuration.
   */
  resolveDebugConfiguration(folder: WorkspaceFolder|undefined, config: DebugConfiguration, token?: CancellationToken):
      ProviderResult<DebugConfiguration> {
    // if launch.json is missing or empty
    if (!config.type && !config.request && !config.name) {
      const editor = vscode.window.activeTextEditor;
      if (editor && editor.document.languageId === 'markdown') {
        config.type = 'mock';
        config.name = 'Launch';
        config.request = 'launch';
        config.program = '${file}';
        config.stopOnEntry = true;
      }
    }

    if (!config.program) {
      return vscode.window.showInformationMessage('Cannot find a program to debug').then(_ => {
        return undefined;  // abort launch
      });
    }

    return config;
  }

  dispose() {
    if (this._server) {
      this._server.close();
    }
  }
}
