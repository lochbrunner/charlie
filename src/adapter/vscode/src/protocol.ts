

export interface Variable {
  name: string;
  value: number;
}

export interface Position {
  line: number;
  column: number;
}

export interface State {
  variable: Variable[];
  callstack_item: number[];
}

export interface Event {
  bytecode: number;
  position: number;
  state: State;
}

export enum Type {
  NEXT_STEP = 0
}

export interface Command { type: Type; }