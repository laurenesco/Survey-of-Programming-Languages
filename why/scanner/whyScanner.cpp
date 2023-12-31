//-----------------------------------------------------------
// Dr. Art Hanna & Lauren Escobedo
// why Scanner
// whyScanner.cpp
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

#include "why.h"

//-----------------------------------------------------------
typedef enum
//-----------------------------------------------------------
{
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

//-----------------------------------------------------------
struct TOKENTABLERECORD
//-----------------------------------------------------------
{
   TOKENTYPE type;
   char description[12 + 1];
   bool isReservedWord;
};

//-----------------------------------------------------------
const TOKENTABLERECORD TOKENTABLE[] =
    //-----------------------------------------------------------
    {
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

//-----------------------------------------------------------
struct TOKEN
//-----------------------------------------------------------
{
   TOKENTYPE type;
   char lexeme[SOURCELINELENGTH + 1];
   int sourceLineNumber;
   int sourceLineIndex;
};

//--------------------------------------------------
// Global variables
//--------------------------------------------------
READER<CALLBACKSUSED> reader(SOURCELINELENGTH, LOOKAHEAD);
LISTER lister(LINESPERPAGE);

//--------------------------------------------------
void ProcessCompilerError(int sourceLineNumber, int sourceLineIndex, const char errorMessage[])
//--------------------------------------------------
{
   char information[SOURCELINELENGTH + 1];

   // Use "panic mode" error recovery technique: report error message and terminate compilation!
   sprintf(information, "     At (%4d:%3d)s", sourceLineNumber, sourceLineIndex, errorMessage);
   lister.ListInformationLine(information);
   lister.ListInformationLine("Compiler ending with compiler error!\n");
   throw(WHY_EXCEPTION("Compiler ending with compiler error!"));
}

//-----------------------------------------------------------
int main()
//-----------------------------------------------------------
{
   void Callback1(int sourceLineNumber, const char sourceLine[]);
   void Callback2(int sourceLineNumber, const char sourceLine[]);
   void GetNextToken(TOKEN tokens[]);

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

      // Scan entire source file (causes outputting of TRACESCANNER-enabled information to list file)
      while (tokens[0].type != stop)
         GetNextToken(tokens);
   }
   catch (WHY_EXCEPTION e)
   {
      cout << "Exception: " << e.GetDescription() << endl;
   }
   lister.ListInformationLine("Scanner ending");
   cout << "Scanner ending\n";

   system("PAUSE");
   return (0);
}

//-----------------------------------------------------------
void Callback1(int sourceLineNumber, const char sourceLine[])
//-----------------------------------------------------------
{
   cout << setw(4) << sourceLineNumber << " ";
}

//-----------------------------------------------------------
void Callback2(int sourceLineNumber, const char sourceLine[])
//-----------------------------------------------------------
{
   cout << sourceLine << endl;
}

//-----------------------------------------------------------
void GetNextToken(TOKEN tokens[])
//-----------------------------------------------------------
{
   const char *TokenDescription(TOKENTYPE type);

   int i;
   TOKENTYPE type;
   char lexeme[SOURCELINELENGTH + 1];
   int sourceLineNumber;
   int sourceLineIndex;
   char information[SOURCELINELENGTH + 1];

   //============================================================
   // Move look-ahead "window" to make room for next token-and-lexeme
   //============================================================
   for (int i = 1; i <= LOOKAHEAD; i++)
      tokens[i - 1] = tokens[i];

   char nextCharacter = reader.GetLookAheadCharacter(0).character;

   //============================================================
   // "Eat" white space and comments
   //============================================================
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

   //============================================================
   // Scan token
   //============================================================
   sourceLineNumber = reader.GetLookAheadCharacter(0).sourceLineNumber;
   sourceLineIndex = reader.GetLookAheadCharacter(0).sourceLineIndex;

   // reserved words (and <identifier> ***BUT NOT YET***)
   if (isalpha(nextCharacter))
   {
      char UCLexeme[SOURCELINELENGTH + 1];

      i = 0;
      lexeme[i++] = nextCharacter;
      nextCharacter = reader.GetNextCharacter().character;
      while (isalpha(nextCharacter) || isdigit(nextCharacter) || (nextCharacter == '_'))
      {
         lexeme[i++] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
      }
      lexeme[i] = '\0';
      for (i = 0; i <= (int)strlen(lexeme); i++)
         UCLexeme[i] = toupper(lexeme[i]);

      bool isFound = false;

      i = 0;
      while (!isFound && (i <= (sizeof(TOKENTABLE) / sizeof(TOKENTABLERECORD)) - 1))
      {
         if (TOKENTABLE[i].isReservedWord && (strcmp(UCLexeme, TOKENTABLE[i].description) == 0))
            isFound = true;
         else
            i++;
      }
      if (isFound)
         type = TOKENTABLE[i].type;
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

//-----------------------------------------------------------
const char *TokenDescription(TOKENTYPE type)
//-----------------------------------------------------------
{
   int i;
   bool isFound;

   isFound = false;
   i = 0;
   while (!isFound && (i <= (sizeof(TOKENTABLE) / sizeof(TOKENTABLERECORD)) - 1))
   {
      if (TOKENTABLE[i].type == type)
         isFound = true;
      else
         i++;
   }
   return (isFound ? TOKENTABLE[i].description : "what even is this");
}
