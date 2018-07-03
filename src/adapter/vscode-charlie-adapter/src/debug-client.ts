import { Breakpoint, BreakpointEvent, Handles, InitializedEvent, Logger, logger, LoggingDebugSession, OutputEvent, Scope, Source, StackFrame, StoppedEvent, TerminatedEvent, Thread } from 'vscode-debugadapter';
import { DebugProtocol } from 'vscode-debugprotocol';

import { ConnectorRuntime } from './connector-runtime';
const { Subject } = require('await-notify');

declare function basename(p: string, ext?: string): string;

/**
 * This interface describes the mock-debug specific launch attributes
 * (which are not part of the Debug Adapter Protocol).
 * The schema for these attributes lives in the package.json of the mock-debug extension.
 * The interface should always match this schema.
 */
interface LaunchRequestArguments extends DebugProtocol.LaunchRequestArguments {
  /** An absolute path to the "program" to debug. */
  program: string;
  hostname: string;
  port: number;
  trace: boolean;
}

export class DebugSession extends LoggingDebugSession {
  private static THREAD_ID = 1;

  private _runtime: ConnectorRuntime;
  private _configurationDone = new Subject();
  private _variableHandles = new Handles<string>();

  public constructor() {
    super('charlie-debug');
    console.log('DebugSession.constructor()');

    // this debugger uses zero-based lines and columns
    this.setDebuggerLinesStartAt1(false);
    this.setDebuggerColumnsStartAt1(false);

    // this._runtime = new ConnectorRuntime();

    // this._runtime.on('stopOnEntry', () => {
    //   this.sendEvent(new StoppedEvent('entry', DebugSession.THREAD_ID));
    // });
    // this._runtime.on('stopOnStep', () => {
    //   this.sendEvent(new StoppedEvent('step', DebugSession.THREAD_ID));
    // });
    // this._runtime.on('stopOnBreakpoint', () => {
    //   this.sendEvent(new StoppedEvent('breakpoint', DebugSession.THREAD_ID));
    // });
    // this._runtime.on('stopOnException', () => {
    //   this.sendEvent(new StoppedEvent('exception', DebugSession.THREAD_ID));
    // });
  }
  /**
   * The 'initialize' request is the first request called by the frontend
   * to interrogate the features the debug adapter provides.
   */
  protected initializeRequest(
    response: DebugProtocol.InitializeResponse, args: DebugProtocol.InitializeRequestArguments): void {
    // build and return the capabilities of this debug adapter:
    response.body = response.body || {};

    // the adapter implements the configurationDoneRequest.
    response.body.supportsConfigurationDoneRequest = true;

    // make VS Code to use 'evaluate' when hovering over source
    response.body.supportsEvaluateForHovers = false;

    // make VS Code to show a 'step back' button
    response.body.supportsStepBack = false;

    this.sendResponse(response);

    // since this debug adapter can accept configuration requests like 'setBreakpoint' at any time,
    // we request them early by sending an 'initializeRequest' to the frontend.
    // The frontend will end the configuration sequence by calling 'configurationDone' request.
    this.sendEvent(new InitializedEvent());
  }

  /**
   * Called at the end of the configuration sequence.
   * Indicates that all breakpoints etc. have been sent to the DA and that the 'launch' can start.
   */
  protected configurationDoneRequest(
    response: DebugProtocol.ConfigurationDoneResponse, args: DebugProtocol.ConfigurationDoneArguments): void {
    super.configurationDoneRequest(response, args);

    // notify the launchRequest that configuration has finished
    this._configurationDone.notify();
  }

  protected async launchRequest(response: DebugProtocol.LaunchResponse, args: LaunchRequestArguments) {
    // make sure to 'Stop' the buffered logging if 'trace' is not set
    logger.setup(args.trace ? Logger.LogLevel.Verbose : Logger.LogLevel.Stop, false);

    // wait until configuration has finished (and configurationDoneRequest has been called)
    await this._configurationDone.wait(1000);

    // start the program in the runtime
    this._runtime.start(args.program, args.hostname, args.port);

    this.sendResponse(response);
  }

  protected setBreakPointsRequest(
    response: DebugProtocol.SetBreakpointsResponse, args: DebugProtocol.SetBreakpointsArguments): void {
    const path = <string>args.source.path;
    const clientLines = args.lines || [];

    // clear all breakpoints for this file
    this._runtime.clearBreakpoints(path);

    // set and verify breakpoint locations
    const actualBreakpoints = clientLines.map(l => {
      let { verified, line, id } = this._runtime.setBreakPoint(path, this.convertClientLineToDebugger(l));
      const bp = <DebugProtocol.Breakpoint>new Breakpoint(verified, this.convertDebuggerLineToClient(line));
      bp.id = id;
      return bp;
    });

    // send back the actual breakpoint positions
    response.body = { breakpoints: actualBreakpoints };
    this.sendResponse(response);
  }

  protected threadsRequest(response: DebugProtocol.ThreadsResponse): void {
    // runtime supports now threads so just return a default thread.
    response.body = { threads: [new Thread(DebugSession.THREAD_ID, 'thread 1')] };
    this.sendResponse(response);
  }

  protected stackTraceRequest(response: DebugProtocol.StackTraceResponse, args: DebugProtocol.StackTraceArguments):
    void {
    const startFrame = typeof args.startFrame === 'number' ? args.startFrame : 0;
    const maxLevels = typeof args.levels === 'number' ? args.levels : 1000;
    const endFrame = startFrame + maxLevels;

    const stk = this._runtime.stack(startFrame, endFrame);

    response.body = {
      stackFrames: stk.frames.map(
        f => new StackFrame(f.index, f.name, this.createSource(f.file), this.convertDebuggerLineToClient(f.line))),
      totalFrames: stk.count
    };
    this.sendResponse(response);
  }

  protected scopesRequest(response: DebugProtocol.ScopesResponse, args: DebugProtocol.ScopesArguments): void {
    const scopes = this._runtime.scope().map(scope => (new Scope(scope.name, scope.id)));
    response.body = { scopes };
    this.sendResponse(response);
  }

  protected variablesRequest(response: DebugProtocol.VariablesResponse, args: DebugProtocol.VariablesArguments): void {
    const id = this._variableHandles.get(args.variablesReference);
    const variables: Array<DebugProtocol.Variable> =
      this._runtime.variables(parseInt(id)).map(variable => ({
        name: variable.name,
        value: variable.value.toString(),
        type: variable.type,
        variablesReference: 0
      }));

    response.body = { variables };
    this.sendResponse(response);
  }

  protected continueRequest(response: DebugProtocol.ContinueResponse, args: DebugProtocol.ContinueArguments): void {
    this._runtime.continue();
    this.sendResponse(response);
  }

  protected nextRequest(response: DebugProtocol.NextResponse, args: DebugProtocol.NextArguments): void {
    this._runtime.step();
    this.sendResponse(response);
  }

  private createSource(filePath: string): Source {
    return new Source(
      basename(filePath), this.convertDebuggerPathToClient(filePath), undefined, undefined, 'mock-adapter-data');
  }
}