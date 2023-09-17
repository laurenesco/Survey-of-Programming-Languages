//-----------------------------------------------------------
// Dr. Art Hanna & Lauren Escobedo
// wju Parser
// whyParser.cpp
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
#define TRACESCANNER
#define TRACEPARSER

#include "why.h"

typedef enum {
   // pseudo-terminals
   identifier,
   words,
   stop,
   unknown,
   // reserved words
   start,
   done,
   say,
   newl,
   // punctuation
   divider,
   endline,
   // operators
   // ***NONE***
} TOKENTYPE;

struct TOKENTABLERECORD {
   TOKENTYPE type;
   char description[12 + 1];
   bool isReservedWord;
};

const TOKENTABLERECORD TOKENDICT[] = {
        {identifier, "identifier", false},
        {words, "words", false},
        {stop, "stop", false},
        {unknown, "unknown", false},
        {start, "start", true},
        {done, "done", true},
        {say, "say", true},
        {newl, "newl", true},
        {divider, "divider", false},
        {endline, "endline", false}};

struct TOKEN {
   TOKENTYPE type;
   char lexeme[SOURCELINELENGTH + 1];
   int sourceLineNumber;
   int sourceLineIndex;
};

// Global variables
READER<CALLBACKSUSED> reader(SOURCELINELENGTH, LOOKAHEAD);
LISTER lister(LINESPERPAGE);

#ifdef TRACEPARSER
int level;
#endif

int main() {
   void Callback1(int sourceLineNumber, const char sourceLine[]);
   void Callback2(int sourceLineNumber, const char sourceLine[]);
   void GetNextToken(TOKEN tokens[]);
   void ParseSPLProgram(TOKEN tokens[]);

   char sourceFileName[80 + 1];
   TOKEN tokens[LOOKAHEAD + 1];

   cout << "Filename: ";
   cin >> sourceFileName;

   try
   {
      lister.OpenFile(sourceFileName);
      reader.SetLister(&lister);
      reader.AddCallbackFunction(Callback1);
      reader.AddCallbackFunction(Callback2);
      reader.OpenFile(sourceFileName);

      // Fill tokens[] for look-ahead
      for (int i = 0; i <= LOOKAHEAD; i++)
         GetNextToken(tokens);

#ifdef TRACEPARSER
      level = 0;
#endif

      ParseSPLProgram(tokens);
   }
   catch (WHY_EXCEPTION e)
   {
      cout << "Exception: " << e.GetDescription() << endl;
   }
   lister.ListInformationLine("Parser ending");
   cout << "Parser ending\n";

   system("PAUSE");
   return (0);
}

void EnterModule(const char module[]) {
#ifdef TRACEPARSER
   char information[SOURCELINELENGTH + 1];

   level++;
   sprintf(information, "   %*s>%s", level * 2, " ", module);
   lister.ListInformationLine(information);
#endif
}

void ExitModule(const char module[]) {
#ifdef TRACEPARSER
   char information[SOURCELINELENGTH + 1];

   sprintf(information, "   %*s<%s", level * 2, " ", module);
   lister.ListInformationLine(information);
   level--;
#endif
}

void ProcessCompilerError(int sourceLineNumber, int sourceLineIndex, const char errorMessage[]) {
   char information[SOURCELINELENGTH + 1];

   // Use "panic mode" error recovery technique: report error message and terminate compilation!
   sprintf(information, "At (%4d:%3d) %s", sourceLineNumber, sourceLineIndex, errorMessage);
   lister.ListInformationLine(information);
   lister.ListInformationLine("Compiler ending with compiler error!\n");
   throw(WHY_EXCEPTION("Compiler ending with compiler error!"));
}

void ParseSPLProgram(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);
   void ParsestartDefinition(TOKEN tokens[]);

   EnterModule("SPLProgram");

   if (tokens[0].type == start)
      ParsestartDefinition(tokens);
   else
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting start");

   if (tokens[0].type != stop)
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting end-of-program");

   ExitModule("SPLProgram");
}

void ParsestartDefinition(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);
   void ParseStatement(TOKEN tokens[]);

   EnterModule("startDefinition");

   GetNextToken(tokens);

   while (tokens[0].type != stop)
      ParseStatement(tokens);

   GetNextToken(tokens);

   ExitModule("startDefinition");
}

void ParseStatement(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);
   void ParsesayStatement(TOKEN tokens[]);

   EnterModule("Statement");

   switch (tokens[0].type)
   {
   case say:
      ParsesayStatement(tokens);
      break;
   default:
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                           "Expecting beginning-of-statement");
      break;
   }

   ExitModule("Statement");
}

void ParsesayStatement(TOKEN tokens[]) {
   void GetNextToken(TOKEN tokens[]);

   EnterModule("sayStatement");

   do
   {
      GetNextToken(tokens);

      switch (tokens[0].type)
      {
      case words:
         GetNextToken(tokens);
         break;
      case newl:
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
            type = stop;
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
