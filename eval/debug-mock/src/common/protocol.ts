

export interface Variable {
  name: string;
  value: number;
  type: string;
}
export interface Scope {
  name: string;
  variable: Variable[];
}

export interface Location {
  line: number;
  column: number;
  filename: string;
}

export interface State {
  scope: Scope[];
  callstackItem: string[];
}

export enum EventReason {
  ON_ENTRY = 0,
  ON_STEP = 1,
  ON_BREAKPOINT = 2,
  ON_EXCEPTION = 3
}

export interface Event {
  bytecode: number;
  position: Location;
  state: State;
  reason: EventReason;
}

export enum Type {
  NEXT_STEP = 0,
  SET_BREAKPOINT = 1,
  CLEAR_BREAKPOINT = 2,
  CLEAR_BREAKPOINTS = 3,
  RUN = 4,
  QUIT = 5,
  LIST_BREAKPOINTS = 6
}

export interface Command {
  type: Type;
  position?: Location;
}