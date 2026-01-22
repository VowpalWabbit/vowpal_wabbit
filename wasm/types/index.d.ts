// Type declarations for @vowpalwabbit/vowpalwabbit
// See https://github.com/VowpalWabbit/vowpal_wabbit/issues/4697

/**
 * Action-score pair representing an action and its associated probability/score
 */
export interface ActionScore {
  action: number;
  score: number;
}

/**
 * Label for a contextual bandit example
 */
export interface CBLabel {
  action: number;
  cost: number;
  probability: number;
}

/**
 * Result from sampling a probability mass function
 */
export interface SampleResult {
  action: number;
  score: number;
  uuid: string;
}

/**
 * CB example with text context and optional labels
 */
export interface CBExample {
  text_context: string;
  labels?: CBLabel[];
}

/**
 * Options for constructing a workspace
 */
export interface WorkspaceOptions {
  /** VW command-line arguments string */
  args_str?: string;
  /** Path to model file (Node.js only) */
  model_file?: string;
  /** Pre-loaded model array [pointer, length] - memory must be allocated via wasmModule._malloc */
  model_array?: [number | undefined, number | undefined];
}

/**
 * Prediction type enum values
 */
export interface PredictionType {
  Scalar: number;
  Scalars: number;
  ActionScores: number;
  Pdf: number;
  ActionProbs: number;
  MultiClass: number;
  MultiLabels: number;
  Prob: number;
  MultiClassProb: number;
  DecisionProbs: number;
  ActionPdfValue: number;
  ActiveMultiClass: number;
}

/**
 * Prediction class with Type enum
 */
export interface Prediction {
  Type: PredictionType;
}

/**
 * Base workspace interface with common methods
 */
export interface WorkspaceBase {
  /**
   * Returns the enum value of the prediction type corresponding to the problem type
   */
  predictionType(): number;

  /**
   * The current total sum of the progressive validation loss
   */
  sumLoss(): number;

  /**
   * Stores the VW model in binary format to a file (Node.js only)
   * @param model_file - Path to save the model
   */
  saveModelToFile(model_file: string): void;

  /**
   * Gets the VW model in binary format as a Uint8Array
   */
  getModelAsArray(): Uint8Array;

  /**
   * Loads a VW model from a file (Node.js only)
   * @param model_file - Path to the model file
   */
  loadModelFromFile(model_file: string): void;

  /**
   * Loads a model from a pre-allocated array in WASM memory
   * @param model_array_ptr - Pointer to model data (allocated via wasmModule._malloc)
   * @param model_array_len - Length of model data
   */
  loadModelFromArray(model_array_ptr: number, model_array_len: number): void;

  /**
   * Deletes the underlying VW instance. Must be called when done.
   */
  delete(): void;
}

/**
 * General-purpose Vowpal Wabbit workspace
 */
export interface Workspace extends WorkspaceBase {
  /**
   * Parse a line of text into a VW example
   * @param line - VW format example string
   */
  parse(line: string): object;

  /**
   * Creates an example from a dense array of features
   * @param features - Map of namespace to feature values
   * @param label - Optional label string
   */
  createExampleFromDense(features: Map<string, number[]>, label?: string): object;

  /**
   * Calls VW predict on the example
   * @param example - Parsed example from parse()
   */
  predict(example: object): any;

  /**
   * Calls VW learn on the example and updates the model
   * @param example - Parsed example from parse()
   */
  learn(example: object): void;

  /**
   * Returns the example to the pool. Must call delete() on example after.
   * @param example - Parsed example from parse()
   */
  finishExample(example: object): void;
}

/**
 * Contextual Bandit workspace for exploration algorithms
 */
export interface CbWorkspace extends WorkspaceBase {
  /**
   * Returns probability mass function over actions
   * @param example - CB example with text_context
   */
  predict(example: CBExample): ActionScore[];

  /**
   * Updates the model with a labeled CB example
   * @param example - CB example with text_context and labels
   */
  learn(example: CBExample): void;

  /**
   * Process a line of CB example text (for file-based learning)
   * @param line - A line from a CB example file
   */
  addLine(line: string): void;

  /**
   * Learn from a full multiline CB example string
   * @param example - Full CB example in VW text format
   */
  learnFromString(example: string): void;

  /**
   * Sample an action from a probability mass function
   * @param pmf - Array of action-score pairs from predict()
   */
  samplePmf(pmf: ActionScore[]): SampleResult;

  /**
   * Sample an action with a specific UUID for reproducibility
   * @param pmf - Array of action-score pairs from predict()
   * @param uuid - UUID to seed the sampling
   */
  samplePmfWithUUID(pmf: ActionScore[], uuid: string): SampleResult;

  /**
   * Predict and sample in one call
   * @param example - CB example with text_context
   */
  predictAndSample(example: CBExample): SampleResult;

  /**
   * Predict and sample with a specific UUID
   * @param example - CB example with text_context
   * @param uuid - UUID to seed the sampling
   */
  predictAndSampleWithUUID(example: CBExample, uuid: string): SampleResult;
}

/**
 * Constructor for Workspace class
 */
export interface WorkspaceConstructor {
  new (options?: WorkspaceOptions): Workspace;
}

/**
 * Constructor for CbWorkspace class
 */
export interface CbWorkspaceConstructor {
  new (options?: WorkspaceOptions): CbWorkspace;
}

/**
 * WebAssembly module interface
 */
export interface WasmModule {
  _malloc(size: number): number;
  _free(ptr: number): void;
  HEAPU8: Uint8Array;
  [key: string]: any;
}

/**
 * VW module interface (Node.js)
 */
export interface VWModule {
  Workspace: WorkspaceConstructor;
  CbWorkspace: CbWorkspaceConstructor;
  Prediction: Prediction;
  VWExampleLogger: VWExampleLoggerConstructor;
  getExceptionMessage(exception: number): string;
  wasmModule: WasmModule;
}

/**
 * VW module interface (Browser)
 */
export interface VWBrowserModule {
  Workspace: WorkspaceConstructor;
  CbWorkspace: CbWorkspaceConstructor;
  Prediction: Prediction;
  getExceptionMessage(exception: number): string;
  wasmModule: WasmModule;
}

/**
 * Example logger for CB examples (Node.js only)
 */
export interface VWExampleLogger {
  /**
   * Start logging to a file (appends)
   * @param log_file - Path to log file
   */
  startLogStream(log_file: string): void;

  /**
   * Log a line asynchronously
   * @param line - Line to log
   */
  logLineToStream(line: string): void;

  /**
   * Close the log stream
   */
  endLogStream(): Promise<void>;

  /**
   * Log a line synchronously (opens/closes file each call)
   * @param log_file - Path to log file
   * @param line - Line to log
   */
  logLineSync(log_file: string, line: string): void;

  /**
   * Convert a CB example to VW text format
   * @param example - CB example object
   */
  CBExampleToString(example: CBExample): string;

  /**
   * Log a CB example asynchronously
   * @param example - CB example object
   */
  logCBExampleToStream(example: CBExample): void;

  /**
   * Log a CB example synchronously
   * @param log_file - Path to log file
   * @param example - CB example object
   */
  logCBExampleSync(log_file: string, example: CBExample): void;
}

/**
 * Constructor for VWExampleLogger class
 */
export interface VWExampleLoggerConstructor {
  new (): VWExampleLogger;
}

/**
 * Default export for Node.js - Promise that resolves to VWModule
 */
declare const vwNodePromise: Promise<VWModule>;
export default vwNodePromise;

/**
 * Named export for browser - Promise that resolves to VWBrowserModule
 */
export declare const vwPromise: Promise<VWBrowserModule>;
