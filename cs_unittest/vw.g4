grammar vw;

label_simple	: FLOAT;

namespace		: '|' ID? WS feature (WS feature)*;

feature			: STRING ':' FLOAT
				| ':' FLOAT
				| STRING;	

STRING	: [^:]+;

ID		: [A-Za-z] [A-Za-z0-9]*;

FLOAT	: [+-]? [0-9+] '.' [0-9];

WS : [ \t\r\n]+ -> skip ; // skip spaces, tabs, newlines