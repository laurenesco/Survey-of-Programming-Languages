//-----------------------------------------------------------
// Dr. Art Hanna & Lauren Escobedo
// why Compiler
// whyCompiler.cpp
//-----------------------------------------------------------
#include <iostream>
#include <iomanip>

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <vector>

using namespace std;

// #define TRACEREADER
// #define TRACESCANNER
// #define TRACEPARSER
#define TRACECOMPILER

#include "why.h"

typedef enum
{
   // pseudo-terminals
   identifier,
   number,
   words,
   stop,
   unknown,
   // reserved words
   start, // program
   done,  // end
   say,
   newl,
   substitute,
   notor,
   ecxor,
   both,
   nand,
   nat,
   absval,
   yes,
   no,
   var,
   num,
   truth,
   con,
   input,
   // punctuation
   divider,
   endline,
   oparen,
   cloparen,
   colon,
   coloneq,
   // operators
   lt,
   le,
   eq,
   gt,
   ge,
   ne,
   add,
   sub,
   mul,
   divis,
   mod,
   pow,
   inc,
   reduce
} TOKENTYPE;

struct TOKENTABLERECORD
{
   TOKENTYPE type;
   char description[12 + 1];
   bool isReservedWord;
};

const TOKENTABLERECORD TOKENDICT[] = {
    {identifier, "identifier", false},
    {number, "number", false},
    {words, "words", false},
    {stop, "stop", true},
    {unknown, "unknown", false},
    {start, "start", true},
    {done, "done", true},
    {say, "say", true},
    {newl, "newl", true},
    {substitute, "substitute", true},
    {notor, "notor", true},
    {ecxor, "ecxor", true},
    {both, "both", true},
    {nand, "nand", true},
    {nat, "nat", true},
    {absval, "absval", true},
    {yes, "yes", true},
    {no, "no", true},
	 {var, "var", true},
    {num, "num", true},
    {truth, "truth", true},
    {con, "con", true},
    {input, "input", true},
    {divider, "divider", false},
    {endline, "endline", false},
    {oparen, "oparen", false},
    {cloparen, "cloparen", false},
    {colon, "colon", false},
    {coloneq, "coloneq", false},
    {lt, "lt", false},
    {le, "le", false},
    {eq, "eq", false},
    {gt, "gt", false},
    {ge, "ge", false},
    {add, "add", false},
    {sub, "sub", false},
    {mul, "mul", false},
    {divis, "divis", false},
    {mod, "mod", false},
    {pow, "pow", false},
    {inc, "inc", false},
    {reduce, "reduce", false}};

struct TOKEN
{
   TOKENTYPE type;
   char lexeme[SOURCELINELENGTH + 1];
   int sourceLineNumber;
   int sourceLineIndex;
};

// Global variables
READER<CALLBACKSUSED> reader(SOURCELINELENGTH, LOOKAHEAD);
LISTER lister(LINESPERPAGE);
// CODEGENERATION
CODE code;
IDENTIFIERTABLE identifierTable(&lister, MAXIMUMIDENTIFIERS);
// ENDCODEGENERATION

int main()
{
   void Callback1(int sourceLineNumber, const char sourceLine[]);
   void Callback2(int sourceLineNumber, const char sourceLine[]);
   void GetNextToken(TOKEN tokens[]);
   void ParseWhyProgram(TOKEN tokens[]);

   char sourceFileName[80 + 1];
   TOKEN tokens[LOOKAHEAD + 1];

   cout << "Filename: ";
   cin >> sourceFileName;

   try
   {
      lister.OpenFile(sourceFileName);

      code.OpenFile(sourceFileName);

      // CODEGENERATION
      code.EmitBeginningCode(sourceFileName);
      // ENDCODEGENERATION

      reader.SetLister(&lister);
      reader.AddCallbackFunction(Callback1);
      reader.AddCallbackFunction(Callback2);
      reader.OpenFile(sourceFileName);

      // Fill tokens[] for look-ahead
      for (int i = 0; i <= LOOKAHEAD; i++)
         GetNextToken(tokens);

      ParseWhyProgram(tokens);

      // CODEGENERATION
      code.EmitEndingCode();
      // ENDCODEGENERATION
   }
   catch (WHY_EXCEPTION e)
   {
      cout << "Exception: " << e.GetDescription() << endl;
   }
   lister.ListInformationLine("Compiler ending");
   cout << "Compiler ending\n";

   system("PAUSE");
   return (0);
}

void EnterModule(const char module[])
{
}

void ExitModule(const char module[])
{
}

void ProcessCompilerError(int sourceLineNumber, int sourceLineIndex, const char errorMessage[])
{
   char information[SOURCELINELENGTH + 1];

   // Use "panic mode" error recovery technique: report error message and terminate compilation!
   sprintf(information, "At (%4d:%3d) %s", sourceLineNumber, sourceLineIndex, errorMessage);
   lister.ListInformationLine(information);
   lister.ListInformationLine("Compiler ending with compiler error!\n");
   throw(WHY_EXCEPTION("Compiler ending with compiler error!"));
}

void ParseWhyProgram(TOKEN tokens[])
{
   void ParseDataDefinitions(TOKEN tokens[], IDENTIFIERSCOPE identifierScope);
   void GetNextToken(TOKEN tokens[]);
   void ParseStartDefinition(TOKEN tokens[]);

   EnterModule("whyProgram");

   ParseDataDefinitions(tokens, GLOBALSCOPE);

#ifdef TRACECOMPILER
   identifierTable.DisplayTableContents("Contents of identifier table after compilation of global data definitions");
#endif

   if (tokens[0].type == start)
      ParseStartDefinition(tokens);
   else
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting start");

   if (tokens[0].type != done)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting end-of-program");

   ExitModule("whyProgram");
}

//-----------------------------------------------------------
void ParseDataDefinitions(TOKEN tokens[], IDENTIFIERSCOPE identifierScope)
//-----------------------------------------------------------
{
   void GetNextToken(TOKEN tokens[]);

   EnterModule("DataDefinitions");

   while ((tokens[0].type == var) || (tokens[0].type == con))
   {
      switch (tokens[0].type)
      {
      case var:
         do
         {
            char identifier_local[MAXIMUMLENGTHIDENTIFIER + 1];
            char reference[MAXIMUMLENGTHIDENTIFIER + 1];
            DATATYPE datatype;
            bool isInTable;
            int index;

            GetNextToken(tokens);

            if (tokens[0].type != identifier)
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
            strcpy(identifier_local, tokens[0].lexeme);
            GetNextToken(tokens);

            if (tokens[0].type != colon)
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ':'");
            GetNextToken(tokens);

            switch (tokens[0].type)
            {
            case num:
               datatype = numeric;
               break;
            case truth:
               datatype = boolean;
               break;
            default:
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting INT or BOOL");
            }
            GetNextToken(tokens);

            index = identifierTable.GetIndex(identifier_local, isInTable);
            if (isInTable && identifierTable.IsInCurrentScope(index))
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Multiply-defined identifier");

            switch (identifierScope)
            {
            case GLOBALSCOPE:
               // CODEGENERATION
               code.AddRWToStaticData(1, identifier_local, reference);
               // ENDCODEGENERATION
               identifierTable.AddToTable(identifier_local, GLOBAL_VARIABLE, datatype, reference);
               break;
            case PROGRAMMODULESCOPE:
               // CODEGENERATION
               code.AddRWToStaticData(1, identifier_local, reference);
               // ENDCODEGENERATION
               identifierTable.AddToTable(identifier_local, PROGRAMMODULE_VARIABLE, datatype, reference);
               break;
            }
         } while (tokens[0].type == divider);

         if (tokens[0].type != endline)
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '.'");
         GetNextToken(tokens);
         break;
      case con:
         do
         {
            char identifier_local[MAXIMUMLENGTHIDENTIFIER + 1];
            char literal[MAXIMUMLENGTHIDENTIFIER + 1];
            char reference[MAXIMUMLENGTHIDENTIFIER + 1];
            DATATYPE datatype;
            bool isInTable;
            int index;

            GetNextToken(tokens);

            if (tokens[0].type != identifier)
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
            strcpy(identifier_local, tokens[0].lexeme);
            GetNextToken(tokens);

            if (tokens[0].type != colon)
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ':'");
            GetNextToken(tokens);

            switch (tokens[0].type)
            {
            case num:
               datatype = numeric;
               break;
            case truth:
               datatype = boolean;
               break;
            default:
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting INT or BOOL");
            }
            GetNextToken(tokens);

            if (tokens[0].type != coloneq)
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ':='");
            GetNextToken(tokens);

            if ((datatype == numeric) && (tokens[0].type == numeric))
            {
               strcpy(literal, "0D");
               strcat(literal, tokens[0].lexeme);
            }
            else if (((datatype == boolean) && (tokens[0].type == yes)) || ((datatype == boolean) && (tokens[0].type == no)))
               strcpy(literal, tokens[0].lexeme);
            else
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch");
            GetNextToken(tokens);

            index = identifierTable.GetIndex(identifier_local, isInTable);
            if (isInTable && identifierTable.IsInCurrentScope(index))
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Multiply-defined identifier");

            switch (identifierScope)
            {
            case GLOBALSCOPE:
               // CODEGENERATION
               code.AddDWToStaticData(literal, identifier_local, reference);
               // ENDCODEGENERATION
               identifierTable.AddToTable(identifier_local, GLOBAL_CONSTANT, datatype, reference);
               break;
            case PROGRAMMODULESCOPE:
               // CODEGENERATION
               code.AddDWToStaticData(literal, identifier_local, reference);
               // ENDCODEGENERATION
               identifierTable.AddToTable(identifier_local, PROGRAMMODULE_CONSTANT, datatype, reference);
               break;
            }
         } while (tokens[0].type == divider);

         if (tokens[0].type != endline)
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '.'");
         GetNextToken(tokens);
         break;
      }
   }

   ExitModule("DataDefinitions");
}

void ParseStartDefinition(TOKEN tokens[])
{
   void ParseDataDefinitions(TOKEN tokens[], IDENTIFIERSCOPE identifierScope);
   void GetNextToken(TOKEN tokens[]);
   void ParseStatement(TOKEN tokens[]);

   char line[SOURCELINELENGTH + 1];
   char label[SOURCELINELENGTH + 1];
   char reference[SOURCELINELENGTH + 1];

   EnterModule("startDefinition");

   // CODEGENERATION
   code.EmitUnformattedLine("; **** =========");
   sprintf(line, "; **** PROGRAM module (%4d)", tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   code.EmitUnformattedLine("; **** =========");
   code.EmitFormattedLine("PROGRAMMAIN", "EQU", "*");

   code.EmitFormattedLine("", "PUSH", "#RUNTIMESTACK", "set SP");
   code.EmitFormattedLine("", "POPSP");
   code.EmitFormattedLine("", "PUSHA", "STATICDATA", "set SB");
   code.EmitFormattedLine("", "POPSB");
   code.EmitFormattedLine("", "PUSH", "#HEAPBASE", "initialize heap");
   code.EmitFormattedLine("", "PUSH", "#HEAPSIZE");
   code.EmitFormattedLine("", "SVC", "#SVC_INITIALIZE_HEAP");
   sprintf(label, "PROGRAMBODY%04d", code.LabelSuffix());
   code.EmitFormattedLine("", "CALL", label);
   code.AddDSToStaticData("Normal program termination", "", reference);
   code.EmitFormattedLine("", "PUSHA", reference);
   code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
   code.EmitFormattedLine("", "SVC", "#SVC_WRITE_ENDL");
   code.EmitFormattedLine("", "PUSH", "#0D0", "terminate with status = 0");
   code.EmitFormattedLine("", "SVC", "#SVC_TERMINATE");
   code.EmitUnformattedLine("");
   code.EmitFormattedLine(label, "EQU", "*");
   // ENDCODEGENERATION

   GetNextToken(tokens);

   identifierTable.EnterNestedStaticScope();
   ParseDataDefinitions(tokens, PROGRAMMODULESCOPE);

   while (tokens[0].type != done)
      ParseStatement(tokens);

   // CODEGENERATION
   code.EmitFormattedLine("", "RETURN");
   code.EmitUnformattedLine("; **** =========");
   sprintf(line, "; **** END (%4d)", tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   code.EmitUnformattedLine("; **** =========");
   // ENDCODEGENERATION

#ifdef TRACECOMPILER
   identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of PROGRAM module definition");
#endif

   identifierTable.ExitNestedStaticScope();

   GetNextToken(tokens);

   ExitModule("startDefinition");
}

void ParseStatement(TOKEN tokens[])
{
   void ParseInputStatement(TOKEN tokens[]);
   void ParseAssignmentStatement(TOKEN tokens[]);
   void ParseSayStatement(TOKEN tokens[]);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Statement");

   switch (tokens[0].type)
   {
   case say:
      ParseSayStatement(tokens);
      break;
   case input:
      ParseInputStatement(tokens);
      break;
   case identifier:
      ParseAssignmentStatement(tokens);
      break;
   default:
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting beginning-of-statement");
      break;
   }

   ExitModule("Statement");
}

void ParseSayStatement(TOKEN tokens[])
{
   void ParseExpression(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH + 1];
   DATATYPE datatype;

   EnterModule("sayStatement");

   // CODEGENERATION
   sprintf(line, "; **** PRINT statement (%4d)", tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   // ENDCODEGENERATION

   do
   {
      GetNextToken(tokens);

      switch (tokens[0].type)
      {
      case words:

         // CODEGENERATION
         char reference[SOURCELINELENGTH + 1];

         code.AddDSToStaticData(tokens[0].lexeme, "", reference);
         code.EmitFormattedLine("", "PUSHA", reference);
         code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
         // ENDCODEGENERATION

         GetNextToken(tokens);
         break;
      case newl:

         // CODEGENERATION
         code.EmitFormattedLine("", "SVC", "#SVC_WRITE_ENDL");
         // ENDCODEGENERATION

         GetNextToken(tokens);
         break;
      default:
         ParseExpression(tokens, datatype);

         // CODEGENERATION
         switch (datatype)
         {
         case numeric:
            code.EmitFormattedLine("", "SVC", "#SVC_WRITE_INTEGER");
            break;
         case boolean:
            code.EmitFormattedLine("", "SVC", "#SVC_WRITE_BOOLEAN");
            break;
         }
         // ENDCODEGENERATION
      }
   } while (tokens[0].type == divider);

   if (tokens[0].type != endline)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting '.'");

   GetNextToken(tokens);

   ExitModule("sayStatement");
}

void ParseInputStatement(TOKEN tokens[])
{
   void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   char reference[SOURCELINELENGTH + 1];
   char line[SOURCELINELENGTH + 1];
   DATATYPE datatype;

   EnterModule("INPUTStatement");

   sprintf(line, "; **** INPUT statement (%4d)", tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   GetNextToken(tokens);

   if (tokens[0].type == words)
   {

      // CODEGENERATION
      code.AddDSToStaticData(tokens[0].lexeme, "", reference);
      code.EmitFormattedLine("", "PUSHA", reference);
      code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
      // ENDCODEGENERATION

      GetNextToken(tokens);
   }

   ParseVariable(tokens, true, datatype);

   // CODEGENERATION
   switch (datatype)
   {
   case numeric:
      code.EmitFormattedLine("", "SVC", "#SVC_READ_INTEGER");
      break;
   case boolean:
      code.EmitFormattedLine("", "SVC", "#SVC_READ_BOOLEAN");
      break;
   }
   code.EmitFormattedLine("", "POP", "@SP:0D1");
   code.EmitFormattedLine("", "DISCARD", "#0D1");
   // ENDCODEGENERATION

   if (tokens[0].type != endline)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '.'");

   GetNextToken(tokens);

   ExitModule("INPUTStatement");
}

void ParseAssignmentStatement(TOKEN tokens[])
{
   void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH + 1];
   DATATYPE datatypeLHS, datatypeRHS;
   int n;

   EnterModule("AssignmentStatement");

   sprintf(line, "; **** assignment statement (%4d)", tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   ParseVariable(tokens, true, datatypeLHS);
   n = 1;

   while (tokens[0].type == divider)
   {
      DATATYPE datatype;

      GetNextToken(tokens);
      ParseVariable(tokens, true, datatype);
      n++;

      if (datatype != datatypeLHS)
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Mixed-mode variables not allowed");
   }
   if (tokens[0].type != coloneq)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ':='");
   GetNextToken(tokens);

   ParseExpression(tokens, datatypeRHS);

   if (datatypeLHS != datatypeRHS)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch");

   // CODEGENERATION
   for (int i = 1; i <= n; i++)
   {
      code.EmitFormattedLine("", "MAKEDUP");
      code.EmitFormattedLine("", "POP", "@SP:0D2");
      code.EmitFormattedLine("", "SWAP");
      code.EmitFormattedLine("", "DISCARD", "#0D1");
   }
   code.EmitFormattedLine("", "DISCARD", "#0D1");
   // ENDCODEGENERATION

   if (tokens[0].type != endline)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '.'");
   GetNextToken(tokens);

   ExitModule("AssignmentStatement");
}

void ParseExpression(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseConjunction(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS, datatypeRHS;

   EnterModule("Expression");

   ParseConjunction(tokens, datatypeLHS);

   if ((tokens[0].type == substitute) ||
       (tokens[0].type == notor) ||
       (tokens[0].type == ecxor))
   {
      while ((tokens[0].type == substitute) ||
             (tokens[0].type == notor) ||
             (tokens[0].type == ecxor))
      {
         TOKENTYPE operation = tokens[0].type;

         GetNextToken(tokens);
         ParseConjunction(tokens, datatypeRHS);
         // CODEGENERATION
         switch (operation)
         {
         case substitute:

            // STATICSEMANTICS
            if (!((datatypeLHS == boolean) && (datatypeRHS == boolean)))
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
            // ENDSTATICSEMANTICS

            code.EmitFormattedLine("", "or");
            datatype = boolean;
            break;
         case notor:

            // STATICSEMANTICS
            if (!((datatypeLHS == boolean) && (datatypeRHS == boolean)))
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
            // ENDSTATICSEMANTICS

            code.EmitFormattedLine("", "nor");
            datatype = boolean;
            break;
         case ecxor:

            // STATICSEMANTICS
            if (!((datatypeLHS == boolean) && (datatypeRHS == boolean)))
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
            // ENDSTATICSEMANTICS

            code.EmitFormattedLine("", "xor");
            datatype = boolean;
            break;
         }
      }
      // CODEGENERATION
   }
   else
      datatype = datatypeLHS;

   ExitModule("Expression");
}

void ParseConjunction(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseNegation(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS, datatypeRHS;

   EnterModule("Conjunction");

   ParseNegation(tokens, datatypeLHS);

   if ((tokens[0].type == both) ||
       (tokens[0].type == nand))
   {
      while ((tokens[0].type == both) ||
             (tokens[0].type == nand))
      {
         TOKENTYPE operation = tokens[0].type;

         GetNextToken(tokens);
         ParseNegation(tokens, datatypeRHS);

         switch (operation)
         {
         case both:
            if (!((datatypeLHS == boolean) && (datatypeRHS == boolean)))
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
            code.EmitFormattedLine("", "and");
            datatype = boolean;
            break;
         case nand:
            if (!((datatypeLHS == boolean) && (datatypeRHS == boolean)))
               ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
            code.EmitFormattedLine("", "nand");
            datatype = boolean;
            break;
         }
      }
   }
   else
      datatype = datatypeLHS;

   ExitModule("Conjunction");
}

void ParseNegation(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseComparison(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeRHS;

   EnterModule("Negation");

   if (tokens[0].type == nat)
   {
      GetNextToken(tokens);
      ParseComparison(tokens, datatypeRHS);

      if (!(datatypeRHS == boolean))
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operand");
      code.EmitFormattedLine("", "not");
      datatype = boolean;
   }
   else
      ParseComparison(tokens, datatype);

   ExitModule("Negation");
}

void ParseComparison(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseComparator(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS, datatypeRHS;

   EnterModule("Comparison");

   ParseComparator(tokens, datatypeLHS);
   if ((tokens[0].type == lt) ||
       (tokens[0].type == le) ||
       (tokens[0].type == eq) ||
       (tokens[0].type == gt) ||
       (tokens[0].type == ge) ||
       (tokens[0].type == ne))
   {
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      ParseComparator(tokens, datatypeRHS);

      if ((datatypeLHS != numeric) || (datatypeRHS != numeric))
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");

      char Tlabel[SOURCELINELENGTH + 1], Elabel[SOURCELINELENGTH + 1];

      code.EmitFormattedLine("", "CMPI");
      sprintf(Tlabel, "T%04d", code.LabelSuffix());
      sprintf(Elabel, "E%04d", code.LabelSuffix());

      switch (operation)
      {
      case lt:
         code.EmitFormattedLine("", "JMPL", Tlabel);
         break;
      case le:
         code.EmitFormattedLine("", "JMPLE", Tlabel);
         break;
      case eq:
         code.EmitFormattedLine("", "JMPE", Tlabel);
         break;
      case gt:
         code.EmitFormattedLine("", "JMPG", Tlabel);
         break;
      case ge:
         code.EmitFormattedLine("", "JMPGE", Tlabel);
         break;
      case ne:
         code.EmitFormattedLine("", "JMPNE", Tlabel);
         break;
      }
      datatype = boolean;
      code.EmitFormattedLine("", "PUSH", "#0X0000");
      code.EmitFormattedLine("", "JMP", Elabel);
      code.EmitFormattedLine(Tlabel, "PUSH", "#0XFFFF");
      code.EmitFormattedLine(Elabel, "EQU", "*");
   }
   else
      datatype = datatypeLHS;

   ExitModule("Comparison");
}

void ParseComparator(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseTerm(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS, datatypeRHS;

   EnterModule("Comparator");

   ParseTerm(tokens, datatypeLHS);

   if ((tokens[0].type == add) ||
       (tokens[0].type == sub))
   {
      while ((tokens[0].type == add) ||
             (tokens[0].type == sub))
      {
         TOKENTYPE operation = tokens[0].type;

         GetNextToken(tokens);
         ParseTerm(tokens, datatypeRHS);

         if ((datatypeLHS != numeric) || (datatypeRHS != numeric))
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");

         switch (operation)
         {
         case add:
            code.EmitFormattedLine("", "ADDI");
            break;
         case sub:
            code.EmitFormattedLine("", "SUBI");
            break;
         }
         datatype = numeric;
      }
   }
   else
      datatype = datatypeLHS;

   ExitModule("Comparator");
}

void ParseTerm(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseFactor(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS, datatypeRHS;

   EnterModule("Term");

   ParseFactor(tokens, datatypeLHS);
   if ((tokens[0].type == mul) ||
       (tokens[0].type == divis) ||
       (tokens[0].type == mod))
   {
      while ((tokens[0].type == mul) ||
             (tokens[0].type == divis) ||
             (tokens[0].type == mod))
      {
         TOKENTYPE operation = tokens[0].type;

         GetNextToken(tokens);
         ParseFactor(tokens, datatypeRHS);

         if ((datatypeLHS != numeric) || (datatypeRHS != numeric))
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");

         switch (operation)
         {
         case mul:
            code.EmitFormattedLine("", "MULI");
            break;
         case divis:
            code.EmitFormattedLine("", "DIVI");
            break;
         case mod:
            code.EmitFormattedLine("", "REMI");
            break;
         }
         datatype = numeric;
      }
   }
   else
      datatype = datatypeLHS;

   ExitModule("Term");
}

void ParseFactor(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseSecondary(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Factor");

   if ((tokens[0].type == absval) ||
       (tokens[0].type == add) ||
       (tokens[0].type == sub))
   {
      DATATYPE datatypeRHS;
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      ParseSecondary(tokens, datatypeRHS);

      if (datatypeRHS != numeric)
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operand");

      switch (operation)
      {
      case absval:
         /*
               SETNZPI
               JMPNN     E????
               NEGI                    ; NEGI or NEGF (as required)
         E???? EQU       *
         */
         {
            char Elabel[SOURCELINELENGTH + 1];

            sprintf(Elabel, "E%04d", code.LabelSuffix());
            code.EmitFormattedLine("", "SETNZPI");
            code.EmitFormattedLine("", "JMPNN", Elabel);
            code.EmitFormattedLine("", "NEGI");
            code.EmitFormattedLine(Elabel, "EQU", "*");
         }
         break;
      case add:
         // Do nothing (identity operator)
         break;
      case sub:
         code.EmitFormattedLine("", "NEGI");
         break;
      }
      datatype = numeric;
   }
   else
      ParseSecondary(tokens, datatype);

   ExitModule("Factor");
}

void ParseSecondary(TOKEN tokens[], DATATYPE &datatype)
{
   void ParsePrefix(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS, datatypeRHS;

   EnterModule("Secondary");

   ParsePrefix(tokens, datatypeLHS);

   if (tokens[0].type == pow)
   {
      GetNextToken(tokens);

      ParsePrefix(tokens, datatypeRHS);

      if ((datatypeLHS != numeric) || (datatypeRHS != numeric))
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");

      code.EmitFormattedLine("", "POWI");
      datatype = numeric;
   }
   else
      datatype = datatypeLHS;

   ExitModule("Secondary");
}

void ParsePrefix(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
   void ParsePrimary(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Prefix");

   if ((tokens[0].type == inc) ||
       (tokens[0].type == reduce))
   {
      DATATYPE datatypeRHS;
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      ParseVariable(tokens, true, datatypeRHS);

      if (datatypeRHS != numeric)
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operand");

      switch (operation)
      {
      case inc:
         code.EmitFormattedLine("", "PUSH", "@SP:0D0");
         code.EmitFormattedLine("", "PUSH", "#0D1");
         code.EmitFormattedLine("", "ADDI");
         code.EmitFormattedLine("", "POP", "@SP:0D1"); // side-effect
         code.EmitFormattedLine("", "PUSH", "@SP:0D0");
         code.EmitFormattedLine("", "SWAP");
         code.EmitFormattedLine("", "DISCARD", "#0D1"); // value
         break;
      case reduce:
         code.EmitFormattedLine("", "PUSH", "@SP:0D0");
         code.EmitFormattedLine("", "PUSH", "#0D1");
         code.EmitFormattedLine("", "SUBI");
         code.EmitFormattedLine("", "POP", "@SP:0D1"); // side-effect
         code.EmitFormattedLine("", "PUSH", "@SP:0D0");
         code.EmitFormattedLine("", "SWAP");
         code.EmitFormattedLine("", "DISCARD", "#0D1"); // value
         break;
      }
      datatype = numeric;
   }
   else
      ParsePrimary(tokens, datatype);

   ExitModule("Prefix");
}

void ParsePrimary(TOKEN tokens[], DATATYPE &datatype)
{
   void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Primary");

   switch (tokens[0].type)
   {
   case number:
   {
      char operand[SOURCELINELENGTH + 1];

      sprintf(operand, "#0D%s", tokens[0].lexeme);
      code.EmitFormattedLine("", "PUSH", operand);
      datatype = numeric;
      GetNextToken(tokens);
   }
   break;
   case yes:
      code.EmitFormattedLine("", "PUSH", "#0XFFFF");
      datatype = boolean;
      GetNextToken(tokens);
      break;
   case no:
      code.EmitFormattedLine("", "PUSH", "#0X0000");
      datatype = boolean;
      GetNextToken(tokens);
      break;
   case oparen:
      GetNextToken(tokens);
      ParseExpression(tokens, datatype);
      if (tokens[0].type != cloparen)
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting )");
      GetNextToken(tokens);
      break;
   case identifier:
      ParseVariable(tokens, false, datatype);
      break;
   default:
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer, true, false, or (");
      break;
   }

   ExitModule("Primary");
}

void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype)
{
   void GetNextToken(TOKEN tokens[]);

   bool isInTable;
   int index;
   IDENTIFIERTYPE identifierType;

   EnterModule("Variable");

   if (tokens[0].type != identifier)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");

   // STATICSEMANTICS
   index = identifierTable.GetIndex(tokens[0].lexeme, isInTable);
   if (!isInTable)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Undefined identifier");

   identifierType = identifierTable.GetType(index);
   datatype = identifierTable.GetDatatype(index);

   if (!((identifierType == GLOBAL_VARIABLE) ||
         (identifierType == GLOBAL_CONSTANT) ||
         (identifierType == PROGRAMMODULE_VARIABLE) ||
         (identifierType == PROGRAMMODULE_CONSTANT)))
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting variable or constant identifier");

   if (asLValue && ((identifierType == GLOBAL_CONSTANT) || (identifierType == PROGRAMMODULE_CONSTANT)))
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Constant may not be l-value");
   // ENDSTATICSEMANTICS

   // CODEGENERATION
   if (asLValue)
      code.EmitFormattedLine("", "PUSHA", identifierTable.GetReference(index));
   else
      code.EmitFormattedLine("", "PUSH", identifierTable.GetReference(index));
   // ENDCODEGENERATION

   GetNextToken(tokens);

   ExitModule("Variable");
}

void Callback1(int sourceLineNumber, const char sourceLine[])
{
   cout << setw(4) << sourceLineNumber << " ";
}

void Callback2(int sourceLineNumber, const char sourceLine[])
{
   cout << sourceLine << endl;

   char line[SOURCELINELENGTH + 1];

   // CODEGENERATION
   sprintf(line, "; %4d %s", sourceLineNumber, sourceLine);
   code.EmitUnformattedLine(line);
   // ENDCODEGENERATION
}

void GetNextToken(TOKEN tokens[])
{
   const char *TokenDescription(TOKENTYPE type);

   int i;
   TOKENTYPE type;
   char lexeme[SOURCELINELENGTH + 1];
   int sourceLineNumber;
   int sourceLineIndex;
   char information[SOURCELINELENGTH + 1];

   // Move look-ahead "window" to make room for next token-and-lexeme
   for (int i = 1; i <= LOOKAHEAD; i++)
      tokens[i - 1] = tokens[i];

   char nextCharacter = reader.GetLookAheadCharacter(0).character;

   // "Eat" white space and comments
   do
   {
      // "Eat" any white-space (blanks and EOLCs and TABCs)
      while ((nextCharacter == ' ') || (nextCharacter == READER<CALLBACKSUSED>::EOLC) || (nextCharacter == READER<CALLBACKSUSED>::TABC))
         nextCharacter = reader.GetNextCharacter().character;

      // "Eat" line comment
      if (nextCharacter == '!')
      {

#ifdef TRACESCANNER
         sprintf(information, "At (%4d:%3d) begin line comment",
                 reader.GetLookAheadCharacter(0).sourceLineNumber,
                 reader.GetLookAheadCharacter(0).sourceLineIndex);
         lister.ListInformationLine(information);
#endif

         do
            nextCharacter = reader.GetNextCharacter().character;
         while (nextCharacter != READER<CALLBACKSUSED>::EOLC);
      }

      //    "Eat" block comments (nesting allowed)
      if ((nextCharacter == '-') && (reader.GetLookAheadCharacter(1).character == '='))
      {
         int depth = 0;

         do
         {
            if ((nextCharacter == '-') && (reader.GetLookAheadCharacter(1).character == '='))
            {
               depth++;

#ifdef TRACESCANNER
               sprintf(information, "At (%4d:%3d) begin block comment depth = %d",
                       reader.GetLookAheadCharacter(0).sourceLineNumber,
                       reader.GetLookAheadCharacter(0).sourceLineIndex,
                       depth);
               lister.ListInformationLine(information);
#endif

               nextCharacter = reader.GetNextCharacter().character;
               nextCharacter = reader.GetNextCharacter().character;
            }
            else if ((nextCharacter == '=') && (reader.GetLookAheadCharacter(1).character == '-'))
            {

#ifdef TRACESCANNER
               sprintf(information, "At (%4d:%3d) end block comment depth = %d",
                       reader.GetLookAheadCharacter(0).sourceLineNumber,
                       reader.GetLookAheadCharacter(0).sourceLineIndex,
                       depth);
               lister.ListInformationLine(information);
#endif

               depth--;
               nextCharacter = reader.GetNextCharacter().character;
               nextCharacter = reader.GetNextCharacter().character;
            }
            else
               nextCharacter = reader.GetNextCharacter().character;
         } while ((depth != 0) && (nextCharacter != READER<CALLBACKSUSED>::EOPC));
         if (depth != 0)
            ProcessCompilerError(reader.GetLookAheadCharacter(0).sourceLineNumber,
                                 reader.GetLookAheadCharacter(0).sourceLineIndex,
                                 "Unexpected end-of-program");
      }
   } while ((nextCharacter == ' ') || (nextCharacter == READER<CALLBACKSUSED>::EOLC) || (nextCharacter == READER<CALLBACKSUSED>::TABC) || (nextCharacter == '!') || ((nextCharacter == '-') && (reader.GetLookAheadCharacter(1).character == '=')));

   // Scan token
   sourceLineNumber = reader.GetLookAheadCharacter(0).sourceLineNumber;
   sourceLineIndex = reader.GetLookAheadCharacter(0).sourceLineIndex;

   // reserved words (and <identifier> ***BUT NOT YET***)
   if (isalpha(nextCharacter))
   {
      i = 0;
      lexeme[i++] = nextCharacter;
      nextCharacter = reader.GetNextCharacter().character;
      while (isalpha(nextCharacter) || isdigit(nextCharacter) || (nextCharacter == '_'))
      {
         lexeme[i++] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
      }
      lexeme[i] = '\0';

      bool isFound = false;

      i = 0;
      while (!isFound && (i <= (sizeof(TOKENDICT) / sizeof(TOKENTABLERECORD)) - 1))
      {
         if (TOKENDICT[i].isReservedWord && (strcmp(lexeme, TOKENDICT[i].description) == 0))
            isFound = true;
         else
            i++;
      }
      if (isFound)
         type = TOKENDICT[i].type;
      else
         type = identifier;
   }
   else if (isdigit(nextCharacter))
   {
      // <integer>
      i = 0;
      lexeme[i++] = nextCharacter;
      nextCharacter = reader.GetNextCharacter().character;

      while (isdigit(nextCharacter))
      {
         lexeme[i++] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
      }

      lexeme[i] = '\0';
      type = number;
   }
   else
   {
      switch (nextCharacter)
      {
         // <string>
      case '-': // print
         i = 0;
         nextCharacter = reader.GetNextCharacter().character;
         while ((nextCharacter != '-') && (nextCharacter != READER<CALLBACKSUSED>::EOLC))
         {
            if ((nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '-'))
            {
               lexeme[i++] = nextCharacter;
               nextCharacter = reader.GetNextCharacter().character;
            }
            else if ((nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '\\'))
            {
               lexeme[i++] = nextCharacter;
               nextCharacter = reader.GetNextCharacter().character;
            }
            lexeme[i++] = nextCharacter;
            nextCharacter = reader.GetNextCharacter().character;
         }
         if (nextCharacter == READER<CALLBACKSUSED>::EOLC)
            ProcessCompilerError(sourceLineNumber, sourceLineIndex,
                                 "Invalid string");
         lexeme[i] = '\0';
         type = words;
         reader.GetNextCharacter();
         break;
      case READER<CALLBACKSUSED>::EOPC:
      {
         static int count = 0;

         if (++count > (LOOKAHEAD + 1))
            ProcessCompilerError(sourceLineNumber, sourceLineIndex,
                                 "Unexpected end-of-program");
         else
         {
            type = done;
            reader.GetNextCharacter();
            lexeme[0] = '\0';
         }
      }
      break;
      case '?': // comma
         type = divider;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '=': // period
         type = endline;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '[': // (
         type = oparen;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case ']': // )
         type = cloparen;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
		case '@':
         lexeme[0] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
         if ( nextCharacter == '=' )
         {
            type = coloneq;
            lexeme[1] = nextCharacter; lexeme[2] = '\0';
            reader.GetNextCharacter();
         }
         else
         {
            type = colon;
            lexeme[1] = '\0';
         }
         break;
      case '<': // lt, le
         lexeme[0] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
         if (nextCharacter == '=')
         {
            type = le;
            lexeme[1] = nextCharacter;
            lexeme[2] = '\0';
            reader.GetNextCharacter();
         }
         else
         {
            type = lt;
            lexeme[1] = '\0';
         }
         break;
      case '>': // gt, ge
         lexeme[0] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
         if (nextCharacter == '=')
         {
            type = ge;
            lexeme[1] = nextCharacter;
            lexeme[2] = '\0';
            reader.GetNextCharacter();
         }
         else
         {
            type = gt;
            lexeme[1] = '\0';
         }
         break;
      case ':': // ==
         type = eq;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '~': // !=
         type = ne;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '&': // +
         type = add;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '+': // -
         lexeme[0] = nextCharacter;
            if ( reader.GetLookAheadCharacter(1).character == '+' )
            {
               nextCharacter = reader.GetNextCharacter().character;
               lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
               type = inc;
            }
            else
            {
               type = sub;
               lexeme[0] = nextCharacter;
					lexeme[1] = '\0';
            }
         reader.GetNextCharacter();
         break;
      case '$': // --
         type = reduce;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '/': // *
         type = mul;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '*': // /
         type = divis;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '#': // %
         type = mod;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '^': // pow
         type = pow;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      default: // unktoken
         type = unknown;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      }
   }

   tokens[LOOKAHEAD].type = type;
   strcpy(tokens[LOOKAHEAD].lexeme, lexeme);
   tokens[LOOKAHEAD].sourceLineNumber = sourceLineNumber;
   tokens[LOOKAHEAD].sourceLineIndex = sourceLineIndex;

#ifdef TRACESCANNER
   sprintf(information, "At (%4d:%3d) token = %12s lexeme = |%s|",
           tokens[LOOKAHEAD].sourceLineNumber,
           tokens[LOOKAHEAD].sourceLineIndex,
           TokenDescription(type), lexeme);
   lister.ListInformationLine(information);
#endif
}

const char *TokenDescription(TOKENTYPE type)
{
   int i;
   bool isFound;

   isFound = false;
   i = 0;
   while (!isFound && (i <= (sizeof(TOKENDICT) / sizeof(TOKENTABLERECORD)) - 1))
   {
      if (TOKENDICT[i].type == type)
         isFound = true;
      else
         i++;
   }
   return (isFound ? TOKENDICT[i].description : "what even is this");
}
