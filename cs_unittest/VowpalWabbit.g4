grammar VowpalWabbit;

label_simple	: value=NUMBER (WS initial=NUMBER)? WS;

namespace		: '|' name=STRING? (WS feature)+ WS?;

feature			: index=(STRING | NUMBER) (':' x=NUMBER)?
				| ':' x=NUMBER
				;

// needs more testing
tag				: ('`' STRING)? WS
				| STRING
				;

example			: label_simple tag? namespace (WS namespace)*;

start			: (example NEWLINE)* example (NEWLINE | EOF);

// greedy matching, if same length its matched in order
NUMBER			: INT | FLOAT;

fragment INT	: [+-]? [0-9]+ ([Ee] '-'? [0-9]+)?;
 
fragment FLOAT 	: [+-]? [0-9]* '.' [0-9]+ ([Ee] '-'? [0-9]+)?;

STRING			: ~([:| \t\r\n])+;

WS				: [ \t]+; // skip spaces, tabs

NEWLINE			: '\r' '\n'?;