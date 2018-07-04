/*---------------------------------------------------------
 * Copyright (C) Microsoft Corporation. All rights reserved.
 *--------------------------------------------------------*/

import {basename} from 'path';
import {Breakpoint, BreakpointEvent, InitializedEvent, Logger, logger, LoggingDebugSession, OutputEvent, Scope, Source, StackFrame, StoppedEvent, TerminatedEvent, Thread} from 'vscode-debugadapter/lib/main';
import {DebugProtocol} from 'vscode-debugprotocol/lib/debugProtocol';

import {MockBreakpoint, MockRuntime} from './debug-client';

const {Subject} = require('await-notify');


/**
 * This interface describes the mock-debug specific launch attributes
 * (which are not part of the Debug Adapter Protocol).
 * The schema for these attributes lives in the package.json of the mock-debug extension.
 * The interface should always match this schema.
 */
interface LaunchRequestArguments extends DebugProtocol.LaunchRequestArguments {
  /** An absolute path to the "program" to debug. */
  program: string;
  /** Automatically stop target after launch. If not specified, target does not stop. */
  stopOnEntry?: boolean;
  /** enable logging the Debug Adapter Protocol */
  trace?: boolean;
  /** Hostname of the server. */
  hostname: string;
  /** Port of the server. */
  port: number;
}

export class MockDebugSession extends LoggingDebugSession {
  // we don't support multiple threads, so we can use a hardcoded ID for the default thread
  private static THREAD_ID = 1;

  // a Mock runtime (or debugger)
  private _runtime: MockRuntime;

  // private _variableHandles = new Handles<string>();

  private _configurationDone = new Subject();

  /**
   * Creates a new debug adapter that is used for one debug session.
   * We configure the default implementation of a debug adapter here.
   */
  public constructor() {
    super('mock-debug.txt');

    // this debugger uses zero-based lines and columns
    this.setDebuggerLinesStartAt1(false);
    this.setDebuggerColumnsStartAt1(false);

    this._runtime = new MockRuntime();

    // setup event handlers
    this._runtime.on('stopOnEntry', () => {
      this.sendEvent(new StoppedEvent('entry', MockDebugSession.THREAD_ID));
    });
    this._runtime.on('stopOnStep', () => {
      this.sendEvent(new StoppedEvent('step', MockDebugSession.THREAD_ID));
    });
    this._runtime.on('stopOnBreakpoint', () => {
      this.sendEvent(new StoppedEvent('breakpoint', MockDebugSession.THREAD_ID));
    });
    this._runtime.on('stopOnException', () => {
      this.sendEvent(new StoppedEvent('exception', MockDebugSession.THREAD_ID));
    });
    this._runtime.on('breakpointValidated', (bp: MockBreakpoint) => {
      this.sendEvent(new BreakpointEvent('changed', <DebugProtocol.Breakpoint>{verified: bp.verified, id: bp.id}));
    });
    this._runtime.on('output', (text, filePath, line, column) => {
      const e: DebugProtocol.OutputEvent = new OutputEvent(`${text}\n`);
      e.body.source = this.createSource(filePath);
      e.body.line = this.convertDebuggerLineToClient(line);
      e.body.column = this.convertDebuggerColumnToClient(column);
      this.sendEvent(e);
    });
    this._runtime.on('end', () => {
      this.sendEvent(new TerminatedEvent());
    });
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
    response.body.supportsEvaluateForHovers = true;

    // make VS Code to show a 'step back' button
    response.body.supportsStepBack = true;

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
    this._runtime.start(args.program, !!args.stopOnEntry, args.hostname, args.port);

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
      let {verified, line, id} = this._runtime.setBreakPoint(path, this.convertClientLineToDebugger(l));
      const bp = <DebugProtocol.Breakpoint>new Breakpoint(verified, this.convertDebuggerLineToClient(line));
      bp.id = id;
      return bp;
    });

    // send back the actual breakpoint positions
    response.body = {breakpoints: actualBreakpoints};
    this.sendResponse(response);
  }

  protected threadsRequest(response: DebugProtocol.ThreadsResponse): void {
    // runtime supports now threads so just return a default thread.
    response.body = {threads: [new Thread(MockDebugSession.THREAD_ID, 'thread 1')]};
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
    // Try map scopes on stack

    const scopes: Array<Scope> = this._runtime.scopes.map((s, i) => (new Scope(s.name, i + 1, false)));

    response.body = {scopes: scopes};
    this.sendResponse(response);
  }

  protected variablesRequest(response: DebugProtocol.VariablesResponse, args: DebugProtocol.VariablesArguments): void {
    const {scopes} = this._runtime;

    const variables: Array<DebugProtocol.Variable> = scopes[args.variablesReference - 1].variable.map(
        v => ({name: v.name, type: v.type, value: v.value.toString(), variablesReference: 0}));

    response.body = {variables: variables};
    this.sendResponse(response);
  }

  protected continueRequest(response: DebugProtocol.ContinueResponse, args: DebugProtocol.ContinueArguments): void {
    this._runtime.continue();
    this.sendResponse(response);
  }

  protected reverseContinueRequest(
      response: DebugProtocol.ReverseContinueResponse, args: DebugProtocol.ReverseContinueArguments): void {
    this._runtime.continue();
    this.sendResponse(response);
  }

  protected nextRequest(response: DebugProtocol.NextResponse, args: DebugProtocol.NextArguments): void {
    this._runtime.step();
    this.sendResponse(response);
  }

  protected stepBackRequest(response: DebugProtocol.StepBackResponse, args: DebugProtocol.StepBackArguments): void {
    this._runtime.step();
    this.sendResponse(response);
  }

  protected evaluateRequest(response: DebugProtocol.EvaluateResponse, args: DebugProtocol.EvaluateArguments): void {
    let reply: string|undefined = undefined;

    // Find the variable in the scopes
    const {scopes} = this._runtime;
    reply = scopes.reduce(
        (ps, s) => ps ||
            s.variable.reduce(
                (pv, v) => pv || (v.name === args.expression ? v.value.toString() : undefined), undefined),
        undefined);

    response.body = {
      result: reply ? reply : `evaluate(context: '${args.context}', '${args.expression}')`,
      variablesReference: 0
    };
    this.sendResponse(response);
  }

  //---- helpers

  private createSource(filePath: string): Source {
    return new Source(
        basename(filePath), this.convertDebuggerPathToClient(filePath), undefined, undefined, 'mock-adapter-data');
  }
}
