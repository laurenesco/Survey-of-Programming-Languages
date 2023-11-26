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

typedef enum {
   // pseudo-terminals
   identifier,
	number,
   words,
   stop,
   unknown,
   // reserved words
   start,	// program
   done,		// end
   say,
   newl,
	or,
	nor,
	xor,
	and,
	nand,
	not,
	abs,
	true,
	false,
   // punctuation
   divider,
   endline,
	oparen,
	cloparen,
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
	div,
	mod,
	pow
} TOKENTYPE;

struct TOKENTABLERECORD {
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
			{or, "or", true},
			{xor, "xor", true},
			{and, "and", true},
			{nand, "nand", true},
			{not, "not", true},
			{abs, "abs", true},
			{true, "true", true},
			{false, "false", true},
        	{divider, "divider", false},
        	{endline, "endline", false},
			{oparen, "oparen", false},
			{cloparen, "cloparen", false},
			{lt, "lt", false},
			{le, "le", false},
			{eq, "eq", false},
			{gt, "gt", false},
			{ge, "ge", false},
			{add, "add", false},
			{sub, "sub", false},
			{mul, "mul", false},
			{div, "div", false},
			{mod, "mod", false},
			{pow, "pow", false}
};

struct TOKEN {
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
// ENDCODEGENERATION


int main() {
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

void EnterModule(const char module[]) {
}

void ExitModule(const char module[]) {
}

void ProcessCompilerError(int sourceLineNumber, int sourceLineIndex, const char errorMessage[]) {
   char information[SOURCELINELENGTH + 1];

   // Use "panic mode" error recovery technique: report error message and terminate compilation!
   sprintf(information, "At (%4d:%3d) %s", sourceLineNumber, sourceLineIndex, errorMessage);
   lister.ListInformationLine(information);
   lister.ListInformationLine("Compiler ending with compiler error!\n");
   throw(WHY_EXCEPTION("Compiler ending with compiler error!"));
}

void ParseWhyProgram(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);
   void ParseStartDefinition(TOKEN tokens[]);

   EnterModule("whyProgram");

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

void ParseStartDefinition(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);
   void ParseStatement(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   char label[SOURCELINELENGTH+1];
   char reference[SOURCELINELENGTH+1];

   EnterModule("startDefinition");

// CODEGENERATION
   code.EmitUnformattedLine("; **** =========");
   sprintf(line,"; **** PROGRAM module (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   code.EmitUnformattedLine("; **** =========");
   code.EmitFormattedLine("PROGRAMMAIN","EQU"  ,"*");

   code.EmitFormattedLine("","PUSH" ,"#RUNTIMESTACK","set SP");
   code.EmitFormattedLine("","POPSP");
   code.EmitFormattedLine("","PUSHA","STATICDATA","set SB");
   code.EmitFormattedLine("","POPSB");
   code.EmitFormattedLine("","PUSH","#HEAPBASE","initialize heap");
   code.EmitFormattedLine("","PUSH","#HEAPSIZE");
   code.EmitFormattedLine("","SVC","#SVC_INITIALIZE_HEAP");
   sprintf(label,"PROGRAMBODY%04d",code.LabelSuffix());
   code.EmitFormattedLine("","CALL",label);
   code.AddDSToStaticData("Normal program termination","",reference);
   code.EmitFormattedLine("","PUSHA",reference);
   code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
   code.EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
   code.EmitFormattedLine("","PUSH","#0D0","terminate with status = 0");
   code.EmitFormattedLine("","SVC" ,"#SVC_TERMINATE");
   code.EmitUnformattedLine("");
   code.EmitFormattedLine(label,"EQU","*");
// ENDCODEGENERATION

   GetNextToken(tokens);

   while (tokens[0].type != done)
      ParseStatement(tokens);

// CODEGENERATION
   code.EmitFormattedLine("","RETURN");
   code.EmitUnformattedLine("; **** =========");
   sprintf(line,"; **** END (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   code.EmitUnformattedLine("; **** =========");
// ENDCODEGENERATION

   GetNextToken(tokens);

   ExitModule("startDefinition");
}

void ParseStatement(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);
   void ParseSayStatement(TOKEN tokens[]);

   EnterModule("Statement");

   switch (tokens[0].type)
   {
   case say:
      ParseSayStatement(tokens);
      break;
   default:
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting beginning-of-statement");
      break;
   }

   ExitModule("Statement");
}

void ParseSayStatement(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];

   EnterModule("sayStatement");

// CODEGENERATION
   sprintf(line,"; **** PRINT statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
// ENDCODEGENERATION

   do
   {
      GetNextToken(tokens);

      switch (tokens[0].type)
      {
      case words:

// CODEGENERATION
            char reference[SOURCELINELENGTH+1];

            code.AddDSToStaticData(tokens[0].lexeme,"",reference);
            code.EmitFormattedLine("","PUSHA",reference);
            code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
// ENDCODEGENERATION

         GetNextToken(tokens);
         break;
      case newl:

// CODEGENERATION
            code.EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
// ENDCODEGENERATION

         GetNextToken(tokens);
         break;
      default:
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                              "Expecting words or newl");
      }
   } while (tokens[0].type == divider);

   if (tokens[0].type != endline)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting '.'");

   GetNextToken(tokens);

   ExitModule("sayStatement");
}

void Callback1(int sourceLineNumber, const char sourceLine[]) {
   cout << setw(4) << sourceLineNumber << " ";
}

void Callback2(int sourceLineNumber, const char sourceLine[]) {
   cout << sourceLine << endl;

	char line[SOURCELINELENGTH+1];

// CODEGENERATION
   sprintf(line,"; %4d %s",sourceLineNumber,sourceLine);
   code.EmitUnformattedLine(line);
// ENDCODEGENERATION
}

void GetNextToken(TOKEN tokens[]) {
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
      //    "Eat" any white-space (blanks and EOLCs and TABCs)
      while ((nextCharacter == ' ') || (nextCharacter == READER<CALLBACKSUSED>::EOLC) || (nextCharacter == READER<CALLBACKSUSED>::TABC))
         nextCharacter = reader.GetNextCharacter().character;

      //    "Eat" line comment
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
   else
   {
      switch (nextCharacter)
      {
         // <string>
      case '-':
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
      case '?':
         type = divider;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      case '=':
         type = endline;
         lexeme[0] = nextCharacter;
         lexeme[1] = '\0';
         reader.GetNextCharacter();
         break;
      default:
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

const char *TokenDescription(TOKENTYPE type) {
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
