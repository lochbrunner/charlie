

export interface Variable {
  name: string;
  value: number;
  type: string;
}

export interface Location {
  line: number;
  column: number;
  filename: string;
}

export interface State {
  variable: Variable[];
  callstackItem: string[];
}

export interface Event {
  bytecode: number;
  position: Location;
  state: State;
}

export enum Type {
  NEXT_STEP = 0,
  SET_BREAKPOINT = 1,
  CLEAR_BREAKPOINT = 2,
  RUN = 3,
  QUIT = 4,
  LIST_BREAKPOINTS = 5
}

export interface Command {
  type: Type;
  position?: Location;
}