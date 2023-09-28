// codegen.c
// Alex Porter
//
// Assignment 4 hand out
//

#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "tree.h"
#include "symtable.h"
#include "TurtleYacc.h"





//
// functions to help generate program
//


TURTLE_OPERATION func_to_turtle_op(FUNCTION_TYPE func)
{
	switch (func)
	{
		case FT_COLOR:
			return TURTLE_OPERATION_COLOR;
		case FT_XCOR:
			return TURTLE_OPERATION_XCOR;
		case FT_YCOR:
			return TURTLE_OPERATION_YCOR;
		case FT_RANDOM:
			return TURTLE_OPERATION_RANDOM;
		case FT_HEADING:
			return TURTLE_OPERATION_HEADING;
		default:
			return (TURTLE_OPERATION)-1;
	}
}

TURTLE_OPERATION turtle_cmd_to_turtle_op(TURTLE_CMD cmd)
{
	switch (cmd)
	{
		case CMD_HOME:
			return TURTLE_OPERATION_HOME;
		case CMD_PU:
			return TURTLE_OPERATION_PU;
		case CMD_PD:
			return TURTLE_OPERATION_PD;
		case CMD_FD:
			return TURTLE_OPERATION_FD;
		case CMD_BK:
			return TURTLE_OPERATION_BK;
		case CMD_RT:
			return TURTLE_OPERATION_RT;
		case CMD_LT:
			return TURTLE_OPERATION_LT;
		case CMD_SETC:
			return TURTLE_OPERATION_SETC;
		case CMD_SETXY:
			return TURTLE_OPERATION_SETXY;
		case CMD_SETH:
			return TURTLE_OPERATION_SETH;
		case CMD_SETX:
			return TURTLE_OPERATION_SETX;
		case CMD_SETY:
			return TURTLE_OPERATION_SETY;
		case CMD_HT:
			return TURTLE_OPERATION_HT;
		case CMD_ST:
			return TURTLE_OPERATION_ST;
		default:
			return (TURTLE_OPERATION)-1;
	}
}

COLORS color_type_to_colors(COLOR_TYPE c)
{
	switch(c)
	{
		case BLACK:
			return COLOR_BLACK;
		case WHITE:
			return COLOR_WHITE;
		case ORANGE:
			return COLOR_ORANGE;
		case YELLOW:
			return COLOR_YELLOW;
		case LIME:
			return COLOR_LIME;
		case CYAN:
			return COLOR_CYAN;
		case BLUE:
			return COLOR_BLUE;
		case MAGENTA:
			return COLOR_MAGENTA;
		case RED:
			return COLOR_RED;
		case BROWN:
			return COLOR_BROWN;
		case GREEN:
			return COLOR_GREEN;
		case TURQUOISE:
			return COLOR_TURQUOISE;
		case SKY:
			return COLOR_SKY;
		case VIOLET:
			return COLOR_VIOLET;
		case PINK:
			return COLOR_PINK;
		default:
			return (COLORS)-1;
	}
}



//
// C++ Version Jan 2017
//


// Node classes for generation

void GenBlockTreeNode::GenerateNode(TurtleProgram * program)
{
	// good to go!
	std::list<TreeNode*> children = GetChildren();
	std::list<TreeNode*>::iterator iter;
	for (iter = children.begin(); iter != children.end(); iter++)
	{
		(*iter)->GenerateNode(program);
	}
}

void GenNumberTreeNode::GenerateNode(TurtleProgram * program)
{
	// good to go!
	program->LOAD_R(REGISTER_G1, Value());
	program->PUSH_R(REGISTER_G1);
}

void GenColorNameTreeNode::GenerateNode(TurtleProgram * program)
{
	program->LOAD_R(REGISTER_G1, color_type_to_colors(Color()));
	program->PUSH_R(REGISTER_G1);
}

void GenFunctionTreeNode::GenerateNode(TurtleProgram * program)
{
	if (Function() == FT_RANDOM)
	{
		TreeNode* param1 = FirstChild();
		param1->GenerateNode(program);
		program->POP_R(REGISTER_P1);
	}

	program->TURTLE(func_to_turtle_op(Function()));
	program->PUSH_R(REGISTER_RE);
}

void GenTurtleCmdTreeNode::GenerateNode(TurtleProgram * program)
{
	switch (Command())
	{
	case CMD_FD:
	case CMD_BK:
	case CMD_RT:
	case CMD_LT:
	case CMD_SETC:
	case CMD_SETX:
	case CMD_SETY:
	case CMD_SETH:
	{
		TreeNode * param1 = FirstChild();
		param1->GenerateNode(program);

		program->POP_R(REGISTER_P1);
		program->TURTLE(turtle_cmd_to_turtle_op(Command()));
	}
	break;

	case CMD_HOME:
	case CMD_PD:
	case CMD_PU:
	case CMD_HT:
	case CMD_ST:
		program->TURTLE(turtle_cmd_to_turtle_op(Command()));
		break;

	case CMD_SETXY:
	{
		TreeNode* param1 = FirstChild();
		param1->GenerateNode(program);

		TreeNode* param2 = SecondChild();
		param2->GenerateNode(program);

		program->POP_R(REGISTER_P2);
		program->POP_R(REGISTER_P1);
		program->TURTLE(turtle_cmd_to_turtle_op(Command()));
	}
	break;

	default:
		break;
	}
}

void GenIfTreeNode::GenerateNode(TurtleProgram * program)
{
	//Generate condition
	TreeNode* condition = FirstChild();
	condition->GenerateNode(program);

	//Compare conditional result to false
	program->POP_R(REGISTER_G1);
	program->LOAD_R(REGISTER_G2, 0);
	program->CMP_RR(REGISTER_G1, REGISTER_G2);

	//Conditionally jump over true block
	TurtleProgram::Label* afterTrueBlock = program->LABEL();
	program->JEq(afterTrueBlock);

	//Generate true block
	TreeNode* trueBlock = SecondChild();
	trueBlock->GenerateNode(program);

	//Place label after true block
	program->PlaceLabel(afterTrueBlock);
}

void GenIfElseTreeNode::GenerateNode(TurtleProgram * program)
{
	TreeNode* condition = FirstChild();
	condition->GenerateNode(program);

	program->POP_R(REGISTER_G1);
	program->LOAD_R(REGISTER_G2, 0);
	program->CMP_RR(REGISTER_G1, REGISTER_G2);

	TurtleProgram::Label* toFalseBlock = program->LABEL();
	program->JEq(toFalseBlock);

	TreeNode* trueblock = SecondChild();
	trueblock->GenerateNode(program);

	TurtleProgram::Label* overFalseBlock = program->LABEL();
	program->JMPRe(overFalseBlock);

	program->PlaceLabel(toFalseBlock);

	TreeNode* falseBlock = ThirdChild();
	falseBlock->GenerateNode(program);
}

void GenOperatorTreeNode::GenerateNode(TurtleProgram * program)
{
	TreeNode * lhs = FirstChild();
	TreeNode * rhs = SecondChild();

	// lhs child
	lhs->GenerateNode(program);

	// rhs child
	rhs->GenerateNode(program);

	// pop operands from the stack
	program->POP_R(REGISTER_G2);
	program->POP_R(REGISTER_G1);

	// do the operation
	switch (Operator())
	{
	case OT_PLUS:
		program->ADD_R(REGISTER_G1, REGISTER_G2);
		break;
	case OT_MINUS:
		program->SUB_R(REGISTER_G1, REGISTER_G2);
		break;
	case OT_TIMES:
		program->MUL_R(REGISTER_G1, REGISTER_G2);
		break;
	case OT_DIVIDE:
		program->DIV_R(REGISTER_G1, REGISTER_G2);
		break;

	case OT_EQUALS:
	case OT_LESSTHAN:
	case OT_GREATERTHAN:
		{
			program->CMP_RR(REGISTER_G1, REGISTER_G2);
			program->LOAD_R(REGISTER_G1, 1);
			TurtleProgram::Label* label = program->LABEL();

			switch (Operator())
			{
			case OT_EQUALS:
				program->JEq(label);
				break;
			case OT_LESSTHAN:
				program->JLt(label);
				break;
			case OT_GREATERTHAN:
				program->JGt(label);
				break;
			}

			program->LOAD_R(REGISTER_G1, 0);
			program->PlaceLabel(label);
			break;
		}

	default:
		break;
	}

	// leave result on the stack
	program->PUSH_R(REGISTER_G1);
}

void GenRepeatTreeNode::GenerateNode(TurtleProgram * program)
{
	TreeNode* count = FirstChild();
	count->GenerateNode(program);

	TurtleProgram::Label* loopTop = program->LABEL();
	program->PlaceLabel(loopTop);

	program->PEEK_R(REGISTER_G1);
	program->LOAD_R(REGISTER_G2, 1);
	program->CMP_RR(REGISTER_G1, REGISTER_G2);

	TurtleProgram::Label* loopEnd = program->LABEL();
	program->JLt(loopEnd);

	TreeNode* loopBlock = SecondChild();
	loopBlock->GenerateNode(program);

	program->POP_R(REGISTER_G1);
	program->DEC_R(REGISTER_G1);
	program->PUSH_R(REGISTER_G1);

	program->JMPTo(loopTop);

	program->PlaceLabel(loopEnd);

	program->POP_R(REGISTER_G1);
}

void GenDeclarationTreeNode::GenerateNode(TurtleProgram * program)
{
	// NOTE: nothing is required here
	// declarations are handled in the bison file by adding variable's type to the symbol table
}

void GenVariableTreeNode::GenerateNode(TurtleProgram * program)
{
	if (SymEntry()->Storage() == ST_GLOBAL)
	{
		program->MOVE_RV(REGISTER_G1, SymEntry());
	}
	else
	{
		program->MOVE_RO(REGISTER_G1, SymEntry()->Address());
	}

	program->PUSH_R(REGISTER_G1);
}

void GenAssignmentTreeNode::GenerateNode(TurtleProgram * program)
{
	TreeNode* rhs = SecondChild();
	rhs->GenerateNode(program);

	program->POP_R(REGISTER_G1);
	VariableTreeNode* var = (VariableTreeNode*)FirstChild();
	
	if (var->SymEntry()->Storage() == ST_GLOBAL)
	{
		program->MOVE_VR(var->SymEntry(), REGISTER_G1);
	}
	else
	{
		program->MOVE_OR(var->SymEntry()->Address(), REGISTER_G1);
	}

}

void GenProcDefTreeNode::GenerateNode(TurtleProgram* program)
{
	TurtleProgram::Label* returnLabel = program->LABEL();
	SetReturnLabel(returnLabel);

	TurtleProgram::Label* afterLabel = program->LABEL();
	program->JMPTo(afterLabel);

	TurtleProgram::Label* procStart = program->LABEL();
	SetCallLabel(procStart);
	program->PlaceLabel(procStart);

	TreeNode* params = FirstChild();
	params->GenerateNode(program);
	int numParams = params->GetChildren().size();

	int localScope = LocalScope();
	std::list<SymbolTable::Entry*> locals = 
		SymbolTable::GetInstance()->GetLocalEntriesForScope(localScope);
	int localOfset = 0;

	for (auto iter = locals.begin(); iter != locals.end(); iter++)
	{
		if ((*iter)->Type() == IT_VARIABLE)
		{
			(*iter)->Address() = localOfset;

			localOfset++;
		}
	}

	program->ENTER(localOfset);

	TreeNode* bodyBlock = SecondChild();
	bodyBlock->GenerateNode(program);

	program->PlaceLabel(returnLabel);

	program->LEAVE();

	program->RET(numParams);

	program->PlaceLabel(afterLabel);
}

void GenParamsTreeNode::GenerateNode(TurtleProgram* program)
{
	std::list<TreeNode*> paramsList = GetChildren();
	std::list<TreeNode*>::iterator iter = paramsList.begin();
	int nParams = paramsList.size();

	for (int iParam = 0; iParam < nParams; iParam++)
	{
		int offset = ((-2 - nParams) + iParam);

		VariableTreeNode* paramNode = (VariableTreeNode*)((*iter)->FirstChild());
		paramNode->SymEntry()->Address() = offset;
		iter++;
	}
}

void GenReturnTreeNode::GenerateNode(TurtleProgram* program)
{
	TreeNode* parent = GetParent();
	while (parent != NULL && parent->NodeType() != NT_PROCDEF)
		parent = parent->GetParent();

	if (parent != NULL)
	{
		if (FirstChild() != NULL)
		{
			TreeNode* returnValue = FirstChild();
			returnValue->GenerateNode(program);
			program->POP_R(REGISTER_RE);
		}
		else
		{
			program->LOAD_R(REGISTER_RE, 0);
		}
		program->JMPTo(((GenProcDefTreeNode*)parent)->GetReturnLabel());
	}
}

void GenProcCallTreeNode::GenerateNode(TurtleProgram* program)
{
	TreeNode* args = FirstChild();
	args->GenerateNode(program);

	program->CALL(Label());

	if (ExpectReturnValue())
	{
		program->PUSH_R(REGISTER_RE);
	}
}

void GenArgsTreeNode::GenerateNode(TurtleProgram* program)
{
	for each (auto child in GetChildren())
	{
		child->GenerateNode(program);
	}
}

