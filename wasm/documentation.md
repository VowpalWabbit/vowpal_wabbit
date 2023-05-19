## Classes

<dl>
<dt><a href="#VWExampleLogger">VWExampleLogger</a></dt>
<dd><p>A class that helps facilitate the stringification of Vowpal Wabbit examples, and the logging of Vowpal Wabbit examples to a file.</p>
</dd>
<dt><a href="#Workspace">Workspace</a> ⇐ <code>WorkspaceBase</code></dt>
<dd><p>A Wrapper around the Wowpal Wabbit C++ library.</p>
</dd>
<dt><a href="#CbWorkspace">CbWorkspace</a> ⇐ <code>WorkspaceBase</code></dt>
<dd><p>A Wrapper around the Wowpal Wabbit C++ library for Contextual Bandit exploration algorithms.</p>
</dd>
</dl>

<a name="VWExampleLogger"></a>

## VWExampleLogger
A class that helps facilitate the stringification of Vowpal Wabbit examples, and the logging of Vowpal Wabbit examples to a file.

**Kind**: global class  

* [VWExampleLogger](#VWExampleLogger)
    * [.startLogStream(log_file)](#VWExampleLogger+startLogStream)
    * [.logLineToStream(line)](#VWExampleLogger+logLineToStream)
    * [.endLogStream()](#VWExampleLogger+endLogStream)
    * [.logLineSync(log_file, line)](#VWExampleLogger+logLineSync)
    * [.CBExampleToString(example)](#VWExampleLogger+CBExampleToString) ⇒ <code>string</code>
    * [.logCBExampleToStream(example)](#VWExampleLogger+logCBExampleToStream)
    * [.logCBExampleSync(log_file, example)](#VWExampleLogger+logCBExampleSync)

<a name="VWExampleLogger+startLogStream"></a>

### vwExampleLogger.startLogStream(log_file)
Starts a log stream to the specified file. Any new logs will be appended to the file.

**Kind**: instance method of [<code>VWExampleLogger</code>](#VWExampleLogger)  
**Throws**:

- <code>Error</code> Throws an error if another logging stream has already been started


| Param | Type | Description |
| --- | --- | --- |
| log_file | <code>string</code> | the path to the file where the log will be appended to |

<a name="VWExampleLogger+logLineToStream"></a>

### vwExampleLogger.logLineToStream(line)
Takes a string and appends it to the log file. Line is logged in an asynchronous manner.

**Kind**: instance method of [<code>VWExampleLogger</code>](#VWExampleLogger)  
**Throws**:

- <code>Error</code> Throws an error if no logging stream has been started


| Param | Type | Description |
| --- | --- | --- |
| line | <code>string</code> | the line to be appended to the log file |

<a name="VWExampleLogger+endLogStream"></a>

### vwExampleLogger.endLogStream()
Closes the logging stream. Logs a warning to the console if there is no logging stream active, but does not throw

**Kind**: instance method of [<code>VWExampleLogger</code>](#VWExampleLogger)  
<a name="VWExampleLogger+logLineSync"></a>

### vwExampleLogger.logLineSync(log_file, line)
Takes a string and appends it to the log file. Line is logged in a synchronous manner.
Every call to this function will open a new file handle, append the line and close the file handle.

**Kind**: instance method of [<code>VWExampleLogger</code>](#VWExampleLogger)  
**Throws**:

- <code>Error</code> Throws an error if another logging stream has already been started


| Param | Type | Description |
| --- | --- | --- |
| log_file | <code>string</code> | the path to the file where the log will be appended to |
| line | <code>string</code> | the line to be appended to the log file |

<a name="VWExampleLogger+CBExampleToString"></a>

### vwExampleLogger.CBExampleToString(example) ⇒ <code>string</code>
Takes a CB example and returns the string representation of it

**Kind**: instance method of [<code>VWExampleLogger</code>](#VWExampleLogger)  
**Returns**: <code>string</code> - the string representation of the CB example  
**Throws**:

- <code>Error</code> Throws an error if the example is malformed


| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | a CB example that will be stringified |

<a name="VWExampleLogger+logCBExampleToStream"></a>

### vwExampleLogger.logCBExampleToStream(example)
Takes a CB example, stringifies it by calling CBExampleToString, and appends it to the log file. Line is logged in an asynchronous manner.

**Kind**: instance method of [<code>VWExampleLogger</code>](#VWExampleLogger)  
**Throws**:

- <code>Error</code> Throws an error if no logging stream has been started


| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | a CB example that will be stringified and appended to the log file |

<a name="VWExampleLogger+logCBExampleSync"></a>

### vwExampleLogger.logCBExampleSync(log_file, example)
Takes a CB example, stringifies it by calling CBExampleToString, and appends it to the log file. Example is logged in a synchronous manner.
Every call to this function will open a new file handle, append the line and close the file handle.

**Kind**: instance method of [<code>VWExampleLogger</code>](#VWExampleLogger)  
**Throws**:

- <code>Error</code> Throws an error if another logging stream has already been started


| Param | Type | Description |
| --- | --- | --- |
| log_file | <code>string</code> | the path to the file where the log will be appended to |
| example | <code>object</code> | a CB example that will be stringified and appended to the log file |

<a name="Workspace"></a>

## Workspace ⇐ <code>WorkspaceBase</code>
A Wrapper around the Wowpal Wabbit C++ library.

**Kind**: global class  
**Extends**: <code>WorkspaceBase</code>  

* [Workspace](#Workspace) ⇐ <code>WorkspaceBase</code>
    * [new Workspace([args_str], [model_file], [model_array])](#new_Workspace_new)
    * [.parse(line)](#Workspace+parse) ⇒
    * [.predict(example)](#Workspace+predict) ⇒
    * [.learn(example)](#Workspace+learn)
    * [.finishExample(example)](#Workspace+finishExample)
    * [.predictionType()](#WorkspaceBase+predictionType) ⇒
    * [.sumLoss()](#WorkspaceBase+sumLoss) ⇒ <code>number</code>
    * [.saveModelToFile(model_file)](#WorkspaceBase+saveModelToFile)
    * [.getModelAsArray()](#WorkspaceBase+getModelAsArray) ⇒ <code>Uint8Array</code>
    * [.loadModelFromFile(model_file)](#WorkspaceBase+loadModelFromFile)
    * [.loadModelFromArray(model_array_ptr, model_array_len)](#WorkspaceBase+loadModelFromArray)
    * [.delete()](#WorkspaceBase+delete)

<a name="new_Workspace_new"></a>

### new Workspace([args_str], [model_file], [model_array])
Creates a new Vowpal Wabbit workspace.
Can accept either or both string arguments and a model file.

**Throws**:

- <code>Error</code> Throws an error if:
- no argument is provided
- both string arguments and a model file are provided, and the string arguments and arguments defined in the model clash
- both string arguments and a model array are provided, and the string arguments and arguments defined in the model clash
- both a model file and a model array are provided


| Param | Type | Description |
| --- | --- | --- |
| [args_str] | <code>string</code> | The arguments that are used to initialize Vowpal Wabbit (optional) |
| [model_file] | <code>string</code> | The path to the file where the model will be loaded from (optional) |
| [model_array] | <code>tuple</code> | The pre-loaded model's array pointer and length (optional).  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function. |

<a name="Workspace+parse"></a>

### workspace.parse(line) ⇒
Parse a line of text into a VW example.
The example can then be used for prediction or learning.
finishExample() must be called and then delete() on the example, when it is no longer needed.

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Returns**: a parsed vw example that can be used for prediction or learning  

| Param | Type |
| --- | --- |
| line | <code>string</code> | 

<a name="Workspace+predict"></a>

### workspace.predict(example) ⇒
Calls vw predict on the example and returns the prediction.

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Returns**: the prediction with a type corresponding to the reduction that was used  

| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | returned from parse() |

<a name="Workspace+learn"></a>

### workspace.learn(example)
Calls vw learn on the example and updates the model

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  

| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | returned from parse() |

<a name="Workspace+finishExample"></a>

### workspace.finishExample(example)
Cleans the example and returns it to the pool of available examples. delete() must also be called on the example object

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  

| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | returned from parse() |

<a name="WorkspaceBase+predictionType"></a>

### workspace.predictionType() ⇒
Returns the enum value of the prediction type corresponding to the problem type of the model

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Overrides**: [<code>predictionType</code>](#WorkspaceBase+predictionType)  
**Returns**: enum value of prediction type  
<a name="WorkspaceBase+sumLoss"></a>

### workspace.sumLoss() ⇒ <code>number</code>
The current total sum of the progressive validation loss

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Overrides**: [<code>sumLoss</code>](#WorkspaceBase+sumLoss)  
**Returns**: <code>number</code> - the sum of all losses accumulated by the model  
<a name="WorkspaceBase+saveModelToFile"></a>

### workspace.saveModelToFile(model_file)
Takes a file location and stores the VW model in binary format in the file.

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Overrides**: [<code>saveModelToFile</code>](#WorkspaceBase+saveModelToFile)  

| Param | Type | Description |
| --- | --- | --- |
| model_file | <code>string</code> | the path to the file where the model will be saved |

<a name="WorkspaceBase+getModelAsArray"></a>

### workspace.getModelAsArray() ⇒ <code>Uint8Array</code>
Gets the VW model in binary format as a Uint8Array that can be saved to a file.
There is no need to delete or free the array returned by this function.
If the same array is however used to re-load the model into VW, then the array needs to be stored in wasm memory (see loadModelFromArray)

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Overrides**: [<code>getModelAsArray</code>](#WorkspaceBase+getModelAsArray)  
**Returns**: <code>Uint8Array</code> - the VW model in binary format  
<a name="WorkspaceBase+loadModelFromFile"></a>

### workspace.loadModelFromFile(model_file)
Takes a file location and loads the VW model from the file.

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Overrides**: [<code>loadModelFromFile</code>](#WorkspaceBase+loadModelFromFile)  

| Param | Type | Description |
| --- | --- | --- |
| model_file | <code>string</code> | the path to the file where the model will be loaded from |

<a name="WorkspaceBase+loadModelFromArray"></a>

### workspace.loadModelFromArray(model_array_ptr, model_array_len)
Takes a model in an array binary format and loads it into the VW instance.
The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function.

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Overrides**: [<code>loadModelFromArray</code>](#WorkspaceBase+loadModelFromArray)  

| Param | Type | Description |
| --- | --- | --- |
| model_array_ptr | <code>number</code> | the pre-loaded model's array pointer  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function. |
| model_array_len | <code>number</code> | the pre-loaded model's array length |

<a name="WorkspaceBase+delete"></a>

### workspace.delete()
Deletes the underlying VW instance. This function should be called when the instance is no longer needed.

**Kind**: instance method of [<code>Workspace</code>](#Workspace)  
**Overrides**: [<code>delete</code>](#WorkspaceBase+delete)  
<a name="CbWorkspace"></a>

## CbWorkspace ⇐ <code>WorkspaceBase</code>
A Wrapper around the Wowpal Wabbit C++ library for Contextual Bandit exploration algorithms.

**Kind**: global class  
**Extends**: <code>WorkspaceBase</code>  

* [CbWorkspace](#CbWorkspace) ⇐ <code>WorkspaceBase</code>
    * [.predict(example)](#CbWorkspace+predict) ⇒ <code>array</code>
    * [.learn(example)](#CbWorkspace+learn)
    * [.addLine(line)](#CbWorkspace+addLine)
    * [.learnFromString(example)](#CbWorkspace+learnFromString)
    * [.samplePmf(pmf)](#CbWorkspace+samplePmf) ⇒ <code>object</code>
    * [.samplePmfWithUUID(pmf, uuid)](#CbWorkspace+samplePmfWithUUID) ⇒ <code>object</code>
    * [.predictAndSample(example)](#CbWorkspace+predictAndSample) ⇒ <code>object</code>
    * [.predictAndSampleWithUUID(example)](#CbWorkspace+predictAndSampleWithUUID) ⇒ <code>object</code>
    * [.predictionType()](#WorkspaceBase+predictionType) ⇒
    * [.sumLoss()](#WorkspaceBase+sumLoss) ⇒ <code>number</code>
    * [.saveModelToFile(model_file)](#WorkspaceBase+saveModelToFile)
    * [.getModelAsArray()](#WorkspaceBase+getModelAsArray) ⇒ <code>Uint8Array</code>
    * [.loadModelFromFile(model_file)](#WorkspaceBase+loadModelFromFile)
    * [.loadModelFromArray(model_array_ptr, model_array_len)](#WorkspaceBase+loadModelFromArray)
    * [.delete()](#WorkspaceBase+delete)

<a name="CbWorkspace+predict"></a>

### cbWorkspace.predict(example) ⇒ <code>array</code>
Takes a CB example and returns an array of (action, score) pairs, representing the probability mass function over the available actions
The returned pmf can be used with samplePmf to sample an action

Example must have the following properties:
- text_context: a string representing the context

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Returns**: <code>array</code> - probability mass function, an array of action,score pairs that was returned by predict  

| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | the example object that will be used for prediction |

<a name="CbWorkspace+learn"></a>

### cbWorkspace.learn(example)
Takes a CB example and uses it to update the model

Example must have the following properties:
- text_context: a string representing the context
- labels: an array of label objects (usually one), each label object must have the following properties:
 - action: the action index
 - cost: the cost of the action
 - probability: the probability of the action

A label object should have more than one labels only if a reduction that accepts multiple labels was used (e.g. graph_feedback)

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  

| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | the example object that will be used for prediction |

<a name="CbWorkspace+addLine"></a>

### cbWorkspace.addLine(line)
Accepts a CB example (in text format) line by line. Once a full CB example is passed in it will call learnFromString.
This is intended to be used with files that have CB examples, that were logged using logCBExampleToStream and are being read line by line.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  

| Param | Type | Description |
| --- | --- | --- |
| line | <code>string</code> | a string representing a line from a CB example in text Vowpal Wabbit format |

<a name="CbWorkspace+learnFromString"></a>

### cbWorkspace.learnFromString(example)
Takes a full multiline CB example in text format and uses it to update the model. This is intended to be used with examples that are logged to a file using logCBExampleToStream.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Throws**:

- <code>Error</code> Throws an error if the example is an object with a label and/or a text_context


| Param | Type | Description |
| --- | --- | --- |
| example | <code>string</code> | a string representing the CB example in text Vowpal Wabbit format |

<a name="CbWorkspace+samplePmf"></a>

### cbWorkspace.samplePmf(pmf) ⇒ <code>object</code>
Takes an exploration prediction (array of action, score pairs) and returns a single action and score,
along with a unique id that was used to seed the sampling and that can be used to track and reproduce the sampling.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Returns**: <code>object</code> - an object with the following properties:
- action: the action index that was sampled
- score: the score of the action that was sampled
- uuid: the uuid that was passed to the predict function  
**Throws**:

- <code>Error</code> Throws an error if the input is not an array of action,score pairs


| Param | Type | Description |
| --- | --- | --- |
| pmf | <code>array</code> | probability mass function, an array of action,score pairs that was returned by predict |

<a name="CbWorkspace+samplePmfWithUUID"></a>

### cbWorkspace.samplePmfWithUUID(pmf, uuid) ⇒ <code>object</code>
Takes an exploration prediction (array of action, score pairs) and a unique id that is used to seed the sampling,
and returns a single action index and the corresponding score.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Returns**: <code>object</code> - an object with the following properties:
- action: the action index that was sampled
- score: the score of the action that was sampled
- uuid: the uuid that was passed to the predict function  
**Throws**:

- <code>Error</code> Throws an error if the input is not an array of action,score pairs


| Param | Type | Description |
| --- | --- | --- |
| pmf | <code>array</code> | probability mass function, an array of action,score pairs that was returned by predict |
| uuid | <code>string</code> | a unique id that can be used to seed the prediction |

<a name="CbWorkspace+predictAndSample"></a>

### cbWorkspace.predictAndSample(example) ⇒ <code>object</code>
Takes an example with a text_context field and calls predict. The prediction (a probability mass function over the available actions)
will then be sampled from, and only the chosen action index and the corresponding score will be returned,
along with a unique id that was used to seed the sampling and that can be used to track and reproduce the sampling.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Returns**: <code>object</code> - an object with the following properties:
- action: the action index that was sampled
- score: the score of the action that was sampled
- uuid: the uuid that was passed to the predict function  
**Throws**:

- <code>Error</code> if there is no text_context field in the example


| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | an example object containing the context to be used during prediction |

<a name="CbWorkspace+predictAndSampleWithUUID"></a>

### cbWorkspace.predictAndSampleWithUUID(example) ⇒ <code>object</code>
Takes an example with a text_context field and calls predict, and a unique id that is used to seed the sampling.
The prediction (a probability mass function over the available actions) will then be sampled from, and only the chosen action index
and the corresponding score will be returned, along with a unique id that was used to seed the sampling and that can be used to track and reproduce the sampling.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Returns**: <code>object</code> - an object with the following properties:
- action: the action index that was sampled
- score: the score of the action that was sampled
- uuid: the uuid that was passed to the predict function  
**Throws**:

- <code>Error</code> if there is no text_context field in the example


| Param | Type | Description |
| --- | --- | --- |
| example | <code>object</code> | an example object containing the context to be used during prediction |

<a name="WorkspaceBase+predictionType"></a>

### cbWorkspace.predictionType() ⇒
Returns the enum value of the prediction type corresponding to the problem type of the model

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Overrides**: [<code>predictionType</code>](#WorkspaceBase+predictionType)  
**Returns**: enum value of prediction type  
<a name="WorkspaceBase+sumLoss"></a>

### cbWorkspace.sumLoss() ⇒ <code>number</code>
The current total sum of the progressive validation loss

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Overrides**: [<code>sumLoss</code>](#WorkspaceBase+sumLoss)  
**Returns**: <code>number</code> - the sum of all losses accumulated by the model  
<a name="WorkspaceBase+saveModelToFile"></a>

### cbWorkspace.saveModelToFile(model_file)
Takes a file location and stores the VW model in binary format in the file.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Overrides**: [<code>saveModelToFile</code>](#WorkspaceBase+saveModelToFile)  

| Param | Type | Description |
| --- | --- | --- |
| model_file | <code>string</code> | the path to the file where the model will be saved |

<a name="WorkspaceBase+getModelAsArray"></a>

### cbWorkspace.getModelAsArray() ⇒ <code>Uint8Array</code>
Gets the VW model in binary format as a Uint8Array that can be saved to a file.
There is no need to delete or free the array returned by this function.
If the same array is however used to re-load the model into VW, then the array needs to be stored in wasm memory (see loadModelFromArray)

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Overrides**: [<code>getModelAsArray</code>](#WorkspaceBase+getModelAsArray)  
**Returns**: <code>Uint8Array</code> - the VW model in binary format  
<a name="WorkspaceBase+loadModelFromFile"></a>

### cbWorkspace.loadModelFromFile(model_file)
Takes a file location and loads the VW model from the file.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Overrides**: [<code>loadModelFromFile</code>](#WorkspaceBase+loadModelFromFile)  

| Param | Type | Description |
| --- | --- | --- |
| model_file | <code>string</code> | the path to the file where the model will be loaded from |

<a name="WorkspaceBase+loadModelFromArray"></a>

### cbWorkspace.loadModelFromArray(model_array_ptr, model_array_len)
Takes a model in an array binary format and loads it into the VW instance.
The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Overrides**: [<code>loadModelFromArray</code>](#WorkspaceBase+loadModelFromArray)  

| Param | Type | Description |
| --- | --- | --- |
| model_array_ptr | <code>number</code> | the pre-loaded model's array pointer  The memory must be allocated via the WebAssembly module's _malloc function and should later be freed via the _free function. |
| model_array_len | <code>number</code> | the pre-loaded model's array length |

<a name="WorkspaceBase+delete"></a>

### cbWorkspace.delete()
Deletes the underlying VW instance. This function should be called when the instance is no longer needed.

**Kind**: instance method of [<code>CbWorkspace</code>](#CbWorkspace)  
**Overrides**: [<code>delete</code>](#WorkspaceBase+delete)  
