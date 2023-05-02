// Desc: This file contains:
//  - the toolbox for the Blockly editor
//  - the code generator for the Blockly editor
//  - the initialization of the Blockly editor

// Add the behaviors for the custom movement commands (see toolbox XML in index.html)
// using this palette: https://coolors.co/palette/8ecae6-219ebc-023047-ffb703-fb8500
Blockly.Blocks['forward'] = {
  init: function() {
    // this.appendDummyInput().appendField("Forward");
    this.appendValueInput("DISTANCE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("forward");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#fb8500");
    this.setTooltip("units are cm");
    this.setHelpUrl("");
  }
};
Blockly.Blocks['backward'] = {
  init: function() {
    // this.appendDummyInput().appendField("Forward");
    this.appendValueInput("DISTANCE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("backward");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#fb8500");
    this.setTooltip("units are cm");
    this.setHelpUrl("");
  }
};
Blockly.Blocks['right'] = {
  init: function() {
    // this.appendDummyInput().appendField("Forward");
    this.appendValueInput("ANGLE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("right");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#ffb700");
    this.setTooltip("units are degrees");
    this.setHelpUrl("");
  }
};
Blockly.Blocks['left'] = {
  init: function() {
    // this.appendDummyInput().appendField("Forward");
    this.appendValueInput("ANGLE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("left");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#ffb700");
    this.setTooltip("units are degrees");
    this.setHelpUrl("");
  }
};

// Initialize the Lisp generator.

const lispGenerator = new Blockly.Generator('Lisp');
lispGenerator.ORDER_ATOMIC = 0;             // 0 "" ...
lispGenerator.ORDER_COLLECTION = 1;         // tuples, lists, dictionaries
lispGenerator.ORDER_STRING_CONVERSION = 1;  // `expression...`
lispGenerator.ORDER_MEMBER = 2.1;           // . []
lispGenerator.ORDER_FUNCTION_CALL = 2.2;    // ()
lispGenerator.ORDER_EXPONENTIATION = 3;     // **
lispGenerator.ORDER_UNARY_SIGN = 4;         // + -
lispGenerator.ORDER_BITWISE_NOT = 4;        // ~
lispGenerator.ORDER_MULTIPLICATIVE = 5;     // * / // %
lispGenerator.ORDER_ADDITIVE = 6;           // + -
lispGenerator.ORDER_BITWISE_SHIFT = 7;      // << >>
lispGenerator.ORDER_BITWISE_AND = 8;        // &
lispGenerator.ORDER_BITWISE_XOR = 9;        // ^
lispGenerator.ORDER_BITWISE_OR = 10;        // |
lispGenerator.ORDER_RELATIONAL = 11;        // in, not in, is, is not, <, <=, >, >=, <>, !=, ==
lispGenerator.ORDER_LOGICAL_NOT = 12;       // not
lispGenerator.ORDER_LOGICAL_AND = 13;       // and
lispGenerator.ORDER_LOGICAL_OR = 14;        // or
lispGenerator.ORDER_CONDITIONAL = 15;       // if else
lispGenerator.ORDER_LAMBDA = 16;            // lambda
lispGenerator.ORDER_NONE = 99;              // (...)

lispGenerator.indentLevel = 1;
lispGenerator.indent = function() {
  return lispGenerator.INDENT.repeat(lispGenerator.indentLevel);
};

lispGenerator.NAMETYPE = {
  'VARIABLE': Blockly.Names.NAME_TYPE,
  'PROCEDURE': Blockly.Names.PROCEDURE_NAME_TYPE,
  'FUNCTION': Blockly.Names.FUNCTION_NAME_TYPE
};

// TODO: figure out if we need this...
lispGenerator.scrub_ = function(block, code, thisOnly) {
  const nextBlock =
      block.nextConnection && block.nextConnection.targetBlock();
  if (nextBlock && !thisOnly) {
    return code + '\n' + lispGenerator.blockToCode(nextBlock);
  }
  return code;
};

//
// Define the block generators
//
lispGenerator['forward'] = function(block) {
  var distance = lispGenerator.valueToCode(block, 'DISTANCE', lispGenerator.ORDER_ATOMIC);
  if (distance.length == 0) { distance = "0" };
  var code = lispGenerator.indent() + `(move ${distance})\n`;
  return code;
};
lispGenerator['backward'] = function(block) {
  var distance = lispGenerator.valueToCode(block, 'DISTANCE', lispGenerator.ORDER_ATOMIC);
  if (distance.length == 0) { distance = "0" };
  var code = lispGenerator.indent() + `(move -${distance})\n`;
  return code;
};
lispGenerator['right'] = function(block) {
  var angle = lispGenerator.valueToCode(block, 'ANGLE', lispGenerator.ORDER_ATOMIC);
  if (angle.length == 0) { angle = "0" };
  var code = lispGenerator.indent() + `(turn ${angle})\n`;
  return code;
};
lispGenerator['left'] = function(block) {
  var angle = lispGenerator.valueToCode(block, 'ANGLE', lispGenerator.ORDER_ATOMIC);
  if (angle.length == 0) { angle = "0" };
  var code = lispGenerator.indent() + `(turn -${angle})\n`;
  return code;
};
lispGenerator['math_number'] = function(block) {
  const code = String(block.getFieldValue('NUM'));
  return [code, lispGenerator.ORDER_ATOMIC];
};
lispGenerator['variables_get'] = function(block) {
  // Variable getter.
  const code =
      lispGenerator.nameDB_.getName(block.getFieldValue('VAR'),
                                    lispGenerator.NAMETYPE['VARIABLE']);
  return [code, lispGenerator.ORDER_ATOMIC];
};
lispGenerator['variables_set'] = function(block) {
  // Variable setter.
  const argument0 =
      lispGenerator.valueToCode(block, 'VALUE', lispGenerator.ORDER_NONE) || '0';
  const varName =
      lispGenerator.nameDB_.getName(block.getFieldValue('VAR'),
                                    lispGenerator.NAMETYPE['VARIABLE']);
  return lispGenerator.indent() + '(define ' + varName + ' ' + argument0 + ')\n';
};
lispGenerator['math_arithmetic'] = function(block) {
  // Basic arithmetic operators, and power.
  const OPERATORS = {
    'ADD': ['+ ', lispGenerator.ORDER_ADDITIVE],
    'MINUS': ['- ', lispGenerator.ORDER_ADDITIVE],
    'MULTIPLY': ['* ', lispGenerator.ORDER_MULTIPLICATIVE],
    'DIVIDE': ['/ ', lispGenerator.ORDER_MULTIPLICATIVE],
    'POWER': ['^ ', lispGenerator.ORDER_EXPONENTIATION],
  };
  const tuple = OPERATORS[block.getFieldValue('OP')];
  const operator = tuple[0];
  const order = tuple[1];
  const argument0 = lispGenerator.valueToCode(block, 'A', order) || '0';
  const argument1 = lispGenerator.valueToCode(block, 'B', order) || '0';
  const code = '(' + operator + ' ' + argument0 + ' ' + argument1 + ')';
  return [code, order];
};
lispGenerator['controls_repeat_ext'] = function(block) {
  let repeats;
  let code = "";
  if (block.getField('TIMES')) {
    // Internal number.
    repeats = String(parseInt(block.getFieldValue('TIMES'), 10));
  } else {
    // External number.
    repeats = lispGenerator.valueToCode(block, 'TIMES', lispGenerator.ORDER_NONE) || '0';
  }
  /*
  if (stringUtils.isNumber(repeats)) {
    repeats = parseInt(repeats, 10);
  } else {
    repeats = '(floor ' + repeats + ')';
  }
  */
  
  let inner = lispGenerator.statementToCode(block, 'DO');
  const varName = lispGenerator.nameDB_.getName("iii", lispGenerator.NAMETYPE['VARIABLE']);
  code = lispGenerator.indent() + `(define ${varName} ${repeats})\n`;
  code += lispGenerator.indent() + `(while (> ${varName} 0)\n`;
  lispGenerator.indentLevel += 1;
  code += lispGenerator.indent() + '(begin\n';
  lispGenerator.indentLevel += 1;
  code += lispGenerator.indent() + inner;
  code += lispGenerator.indent() + `(setq ${varName} (- ${varName} 1))\n`;
  lispGenerator.indentLevel -= 1;
  code += lispGenerator.indent() + ')\n';
  lispGenerator.indentLevel -= 1;
  code += lispGenerator.indent() + ')\n';
  return code;
};
lispGenerator['controls_repeat'] = lispGenerator['controls_repeat_ext'];

lispGenerator.init = function(workspace) {
  this.workspace_ = workspace;
  this.precedence_ = 0;
  this.fieldSeparator_ = ' ';
  this.statementPrefix_ = '';
  this.INDENT = '  ';
  this.workspace_ = workspace;
  this.allBlocks_ = workspace.getAllBlocks();
  this.loopCount_ = 0;

  // make sure the user doesn't create variable names that will confuse lisp
  this.addReservedWords( ["type", "eval", "quote", "cons", "car", "cdr", "+", "-", "*", "/", "int",
    "<", "eq?", "not", "or", "and", "begin", "while", "cond", "if", "lambda",
    "macro", "define", "assoc", "env", "let", "let*", "letrec", "letrec*",
    "setq", "set-car!", "set-cdr!", "read", "print", "println", "write",
    "string", "load", "trace", "catch", "throw", "quit"].join(",") );

  // initialize the variable database
  if (!this.nameDB_) {
    this.nameDB_ = new Blockly.Names(this.RESERVED_WORDS_);
  } else {
    this.nameDB_.reset();
  }
  this.nameDB_.setVariableMap(workspace.getVariableMap());
  this.nameDB_.populateVariables(workspace);
  this.nameDB_.populateProcedures(workspace);
  
  this.isInitialized = true;
};

function generateCode() {
  return '(begin\n' + lispGenerator.workspaceToCode(workspace) + ')\n';
}

const workspace = Blockly.inject('blocklyDiv', { toolbox: document.getElementById('toolbox') });
lispGenerator.init(workspace);
