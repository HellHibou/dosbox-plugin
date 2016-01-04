/**
 * \file WINRUN.cpp
 * \brief Win16 command line application launcher.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#include <windows.h>
#include <stdlib.h>
#include <shellapi.h>
#include <string.h>
#include <stdio.h>

const char * EXIT_APP_QUESTION = "Program terminated. Exit Windows ?";
const char * EXIT_ERROR_QUESTION = "\nExit Windows ?";

HINSTANCE appHnd      = NULL;
UINT      timerHnd    = NULL;
char *    application = NULL;
char      confirmExit = TRUE;
char      exitAtEnd = FALSE;

int doExit(int returnCode = 0, char * errorMsg = NULL);

VOID CALLBACK timerFct(HWND hwnd, UINT uMsg, UINT id, DWORD dwTime) {
	 if (GetModuleUsage (appHnd) == 0) { doExit(); }
}

int PASCAL WinMain (HINSTANCE hinst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow) {
	 int size;
	 char * workingDir;
	 char * args;
	 char * ptr;

	 size = strlen(cmdline);
	 ptr = (char*) malloc(size + 1);
	 strcpy(ptr, cmdline);
	 cmdline = ptr;

	 while(*ptr == '/') {
		  args = ptr;
		  while((*ptr != 0x00) && (*ptr != ' ')) { ptr++; }
		  if (*ptr != 0x00)  {
				*ptr = 0x00;
				ptr++;
				if (strcmpi (args, "/?") == 0) {
					 MessageBox (NULL,
							  "Command line : [ WIN ] WINRUN [ ARGUMENTS ] < APPLICATION >\n"
							  "\n"
							  "WIN: Launch Windows application from DOS.\n"
							  "Arguments :\n"
							  "     /?: This help screen.\n"
							  "     /min: Run application minimized.\n"
							  "     /max: Run application maximized.\n"
                       "     /exit: Exit Windows at application's end.\n"
							  "     /noconfirm: Exit Windows without confirmation."
							  , "WinRun by Hell Hibou", MB_ICONINFORMATION | MB_OK);

					 return 0;
				}
				else if (strcmpi (args, "/noconfirm") == 0) { confirmExit = FALSE; }
				else if (strcmpi (args, "/min")  == 0) { cmdshow = SW_MINIMIZE; }
				else if (strcmpi (args, "/max")  == 0) { cmdshow = SW_MAXIMIZE; }
				else if (strcmpi (args, "/exit") == 0) { exitAtEnd = TRUE; }
				while((*ptr != 0x00) && (*ptr == ' ')) { ptr++; }
		  }
	 }

	 workingDir = ptr;
	 args = ptr;
	 while ((*args != 0x00) && (*args != ' ')) { args++; };

	 if (*args == ' ') {
		 *args = 0x00;
		 args++;
	 }

	 application = workingDir;
	 ptr = workingDir;

	 while (*ptr != 0x00) {
		if (*ptr == '\\') { application = ptr; }
		ptr++;
	 }

	 if (application == workingDir) { workingDir = NULL; }
	 else {
		  if (*application != 0x00) {
				*application = 0x00;
				application++;
		  }
	 }

	 if (*application == 0x00) { return 0; }
	 appHnd = ShellExecute (NULL, NULL, application, args, workingDir, cmdshow);

	 switch ((WORD)appHnd) {
		  case ERROR_OUTOFMEMORY:
				 return doExit(appHnd, "Out of memory or resources.");

		  case 2L:
				 return doExit(appHnd, "File or application not found");

		  case 3L:
				 return doExit(appHnd, "The path was not found.");

		  case 11L:
				 return doExit(appHnd, "Unsupported application");

		  case ERROR_ACCESS_DENIED:
				 return doExit(appHnd, "Access denied");

		  case SE_ERR_ASSOCINCOMPLETE:
				 return doExit(appHnd, "The file name association is incomplete or invalid.");

		  case SE_ERR_DDEBUSY:
		  case SE_ERR_DDEFAIL:
		  case SE_ERR_DDETIMEOUT:
				 return doExit(appHnd, "The DDE transaction failed.");

		  case SE_ERR_NOASSOC:
				 return doExit(appHnd, "There is no application associated with the given file name extension.");

		  case SE_ERR_SHARE:
				 return doExit(appHnd, "A sharing violation occurred.");

		  default:
				  if ((WORD)appHnd <= 32) return doExit(appHnd);
	 }

	 free (workingDir);
	 if (exitAtEnd == FALSE) { exit (0); }
	 timerHnd = SetTimer(NULL, 0, 1000, (TIMERPROC) timerFct);

	 if (timerHnd == NULL) {
		  while (GetModuleUsage (appHnd) != 0) {
				MSG msg;

				if (GetMessage(&msg, 0, 0, 0))
				{
					 TranslateMessage(&msg);
					 DispatchMessage(&msg);
				}
		  }
	 }
	 else {
		  while (TRUE) {
				MSG msg;

				if (GetMessage(&msg, 0, 0, 0))
				{
					 TranslateMessage(&msg);
					 DispatchMessage(&msg);
				}
		  }
	 }

	 return doExit();
}

int doExit(int returnCode, char * errorMsg)
{
	 if (timerHnd != NULL) KillTimer(NULL, timerHnd);
	 if (exitAtEnd == FALSE) {
		  if (errorMsg != NULL) {
				MessageBox(NULL, errorMsg, application, MB_ICONHAND | MB_OK);
		  }
	 } else if (confirmExit == TRUE) {
		  if (errorMsg == NULL) {
				if (MessageBox(NULL, EXIT_APP_QUESTION, application, MB_YESNO | MB_ICONQUESTION) == IDYES) {
					 ExitWindows(0,0);
				}
		  } else {
				char * msg = (char *) malloc(strlen(errorMsg) + strlen(EXIT_ERROR_QUESTION) + 1);
				strcpy(msg, errorMsg);
				strcat(msg, EXIT_ERROR_QUESTION);
				if (MessageBox(NULL, msg, application, MB_YESNO | MB_ICONHAND) == IDYES) {
					 ExitWindows(0,0);
				}
				free(msg);
		  }
	 } else {
		  if (errorMsg != NULL) {
				MessageBox(NULL, errorMsg, application, MB_ICONHAND | MB_OK);
		  }

		  ExitWindows(0,0);
	 }

	 exit (returnCode);
	 return returnCode;
}
