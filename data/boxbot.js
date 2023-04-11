// Desc: This file contains:
//  - the toolbox for the Blockly editor
//  - the code generator for the Blockly editor
//  - the initialization of the Blockly editor
/* const toolbox = {
  "kind": "categoryToolbox",
  "contents": [
    {
      "kind": "category",
      "name": "Control",
      "contents": [
        {
          "kind": "block",
          "type": "controls_if"
        },
      ]
    },
    {
      "kind": "category",
      "name": "Logic",
      "contents": [
        {
          "kind": "block",
          "type": "logic_compare"
        },
        {
          "kind": "block",
          "type": "logic_operation"
        },
        {
          "kind": "block",
          "type": "logic_boolean"
        }
      ]
    }
  ]
};
 */
const toolbox = {
  "kind": "categoryToolbox",
  "contents": [
    {
      "kind": "category",
      "name": "Logic",
      "colour": "#5C68FF",
      "contents": [
        {
          "kind": "block",
          "type": "logic_compare"
        },
        {
          "kind": "block",
          "type": "logic_operation"
        },
        {
          "kind": "block",
          "type": "logic_negate"
        },
        {
          "kind": "block",
          "type": "logic_boolean"
        },
        {
          "kind": "block",
          "type": "logic_null"
        },
        {
          "kind": "block",
          "type": "logic_ternary"
        }
      ]
    },
    {
      "kind": "category",
      "name": "Control",
      "colour": "#FFAB19",
      "contents": [
        {
          "kind": "block",
          "type": "controls_if"
        },
        {
          "kind": "block",
          "type": "controls_whileUntil"
        },
        {
          "kind": "block",
          "type": "controls_for"
        },
        {
          "kind": "block",
          "type": "controls_flow_statements"
        }
      ]
    },
    {
      "kind": "category",
      "name": "Loops",
      "colour": "#FFBF00",
      "contents": [
        {
          "kind": "block",
          "type": "loops_repeat_ext"
        },
        {
          "kind": "block",
          "type": "controls_repeat_ext"
        },
        {
          "kind": "block",
          "type": "controls_forEach"
        },
        {
          "kind": "block",
          "type": "controls_flow_statements"
        }
      ]
    },
    {
      "kind": "category",
      "name": "Variables",
      "colour": "#AA00FF",
      "custom": "VARIABLE"
    }
  ]
}

const workspace = Blockly.inject('blocklyDiv', {toolbox: toolbox});