
'use strict';

import * as vscode from 'vscode';


// import * as vscodeDebugprotocol from 'vscode-debugadapter';

// import * as net from 'net';

export function activate(context: vscode.ExtensionContext) {
  console.log('activate called');
}

export function deactivate() {
  // nothing to do
}

class MockConfigurationProvider implements vscode.DebugConfigurationProvider {
  resolveDebugConfiguration(folder: vscode.WorkspaceFolder | undefined, config: vscode.DebugConfiguration, token?: vscode.CancellationToken): vscode.ProviderResult<vscode.DebugConfiguration> {
    console.log('resolveDebugConfiguration');
    return config;
  }
}