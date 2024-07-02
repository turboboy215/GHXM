/*GHX (Shin'en GBC) to XM converter*/
/*By Will Trowbridge*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long offset;
long headerOffset;
int i, j;
int trFix = 0;
char outfile[1000000];
long bankAmt;
int numSongs = 0;
int curSong = 0;
int patRows = 0;
long songTable = 0;
long insTable = 0;
long patTable = 0;
long songInfo[4];
unsigned long seqList[500];
char songNames[50][21];
int totalSeqs;

unsigned static char* romData;
unsigned static char* xmData;

/*Bytes to look for before finding GHX "header" - end of vibrato table*/
const char magicBytes[8] = { 0x00, 0xFA, 0xF5, 0xF2, 0xF1, 0xF2, 0xF5, 0xFA };
/*"GHX" - start of "header"*/
const char headerBytes[3] = { 0x47, 0x48, 0x58 };
/*Bytes to check for start of song name list - "SONG"*/
const char songTitle[4] = { 0x53, 0x4F, 0x4E, 0x47 };
const char* noteVals[66] = { "C-4", "C#-4", "D-4", "D#-4", "E-4", "F-4", "F#-4", "G-4", "G#-4", "A-4", "A#-4", "B-4", "C-5", "C#-5", "D-5", "D#-5", "E-5", "F-5", "F#-5", "G-5", "G#-5", "A-5", "A#-5", "B-5", "C-6", "C#-6", "D-6", "D#-6", "E-6", "F-6", "F#-6", "G-6", "G#-6", "A-6", "A#-6", "B-6", "C-7", "C#-7", "D-7", "D#-7", "E-7", "F-7", "F#-7", "G-7", "G#-7", "A-7", "A#-7", "B-7", "C-8", "C#-8", "D-8" };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long info[4]);
void getSeqList(unsigned long list[], long offset);
void getSongTitles(char names[50][21]);
void seqs2txt(unsigned long list[]);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("GHX (Shin'en GBC) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: GHX2TXT <rom> <bank>");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
		}
		fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
		romData = (unsigned char*)malloc(bankSize);
		fread(romData, 1, bankSize, rom);
		fclose(rom);

		for (i = 0; i < bankSize; i++)
		{
			if (!memcmp(&romData[i], magicBytes, 8))
			{
				for (j = i; j < bankSize; j++)
				{
					if (!memcmp(&romData[j], headerBytes, 3))
					{
						headerOffset = j+bankAmt;
						printf("GHX header found at address 0x%04X!\n", headerOffset);
						break;
					}
				}
				if (headerOffset != NULL)
				{
					numSongs = romData[headerOffset - bankAmt + 3];
					if (numSongs == 0)
					{
						numSongs = 1;
					}
					printf("Number of songs: %i\n", numSongs);
					patRows = romData[headerOffset - bankAmt + 4];
					printf("Rows per pattern: %i\n", patRows);
					patTable = ReadLE16(&romData[headerOffset - bankAmt + 6]);
					printf("Sequence data table: 0x%04X\n", patTable);
					insTable = ReadLE16(&romData[headerOffset - bankAmt + 8]);
					printf("Instrument table: 0x%04X\n", insTable);
					songTable = ReadLE16(&romData[headerOffset - bankAmt + 10]);
					printf("Song table: 0x%04X\n", songTable);
					getSeqList(seqList, patTable);
					getSongTitles(songNames);

					i = songTable;
					for (curSong = 1; curSong <= numSongs; curSong++)
					{
						printf("\nSong %i:\n", curSong);
						if (songNames[0][0] != '\0')
						{
							printf("Title: %s\n", songNames[curSong-1]);
						}
						songInfo[0] = romData[i-bankAmt] + 1;
						printf("Number of patterns: %i\n", songInfo[0]);
						songInfo[1] = ReadLE16(&romData[i-bankAmt + 1]);
						printf("Song data address: 0x%04X\n", songInfo[1]);
						songInfo[2] = romData[i-bankAmt + 3] + 1;
						printf("Loop pattern relative to final pattern: %i\n", songInfo[2]);
						songInfo[3] = ReadLE16(&romData[i-bankAmt + 4]);
						printf("Pattern loop address: 0x%04X\n", songInfo[3]);
						if (songInfo[0] < 128)
						{
							song2txt(curSong, songInfo);
						}
						else
						{
							printf("Invalid song, skipping.\n");
						}

						i += 6;
					}
					seqs2txt(seqList);
					
				}
				else
				{
					printf("ERROR: Magic bytes not found!\n");
					exit(-1);
				}

			}
		}
		printf("The operation was successfully completed!\n");
	}
}


void song2txt(int songNum, long info[4])
{
	int curPat = 0;
	long pattern[7];
	long curPos = 0;
	int index = 0;
	int curSeq = 0;
	signed int transpose = 0;

	sprintf(outfile, "song%d.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%d.txt!\n", songNum);
		exit(2);
	}
	else
	{
		curPos = info[1] - bankAmt;
		if (ReadLE16(&romData[curPos]) > headerOffset && ReadLE16(&romData[curPos]) < (headerOffset * 2) && songNum == 1)
		{
			trFix = 1;
		}
		for (curPat = 0; curPat < info[0]; curPat++)
		{
			fprintf(txt, "Pattern %i\n===========================\n", curPat+1);

			/*The typical pattern format*/
			if (trFix == 0)
			{
				for (index = 0; index < 7; index++)
				{
					pattern[index] = romData[curPos + index];
				}
				curSeq = pattern[0];
				fprintf(txt, "Channel 1 sequence: %i\n", curSeq+1);
				transpose = (signed char)pattern[1];
				fprintf(txt, "Channel 1 transpose: %i\n", transpose);
				curSeq = pattern[2];
				fprintf(txt, "Channel 2 sequence: %i\n", curSeq+1);
				transpose = (signed char)pattern[3];
				fprintf(txt, "Channel 2 transpose: %i\n", transpose);
				curSeq = pattern[4];
				fprintf(txt, "Channel 3 sequence: %i\n", curSeq+1);
				transpose = (signed char)pattern[5];
				fprintf(txt, "Channel 3 transpose: %i\n", transpose);
				curSeq = pattern[6];
				fprintf(txt, "Channel 4 sequence: %i\n", curSeq+1);
				curPos += 7;
				fprintf(txt, "\n");
			}
			/*Alternate pattern format used for Tomb Raider*/
			else
			{
				for (index = 0; index < 7; index++)
				{
					if (index == 0 || index == 2 || index == 4 || index == 6)
					{
						pattern[index] = ReadLE16(&romData[curPos + index]);
					}
					else
					{
						pattern[index] = romData[curPos + index];
					}

				}
				curSeq = ReadLE16(&romData[curPos]);
				fprintf(txt, "Channel 1 sequence: 0x%04X\n", curSeq+1);
				transpose = (signed char)romData[curPos + 2];
				fprintf(txt, "Channel 1 transpose: %i\n", transpose);
				curSeq = ReadLE16(&romData[curPos + 3]);
				fprintf(txt, "Channel 2 sequence: 0x%04X\n", curSeq+1);
				transpose = (signed char)romData[curPos + 5];
				fprintf(txt, "Channel 2 transpose: %i\n", transpose);
				curSeq = ReadLE16(&romData[curPos + 6]);
				fprintf(txt, "Channel 3 sequence: 0x%04X\n", curSeq+1);
				transpose = (signed char)romData[curPos + 8];
				fprintf(txt, "Channel 3 transpose: %i\n", transpose);
				curSeq = ReadLE16(&romData[curPos + 9]);
				fprintf(txt, "Channel 4 sequence: 0x%04X\n", curSeq+1);
				curPos += 11;
				fprintf(txt, "\n");
			}


		}
	}
	fclose(txt);

}

void getSeqList(unsigned long list[], long offset)
{
	int cnt = 0;
	unsigned long curValue;
	unsigned long curValue2;
	long newOffset = offset;
	long offset2 = offset - bankAmt;

	for (cnt = 0; cnt < 500; cnt++)
	{
		curValue = (ReadLE16(&romData[newOffset - bankAmt])) - bankAmt;
		curValue2 = (ReadLE16(&romData[newOffset - bankAmt]));
		if (curValue2 >= bankAmt && curValue2 < (bankAmt * 2))
		{
			list[cnt] = curValue;
			newOffset += 2;
		}
		else
		{
			totalSeqs = cnt;
			break;
		}
	}
}

void getSongTitles(char names[50][21])
{
	long curPos = 0;
	long ptrOffset = 0;
	int k, l = 0;
	for (curPos = 0; curPos < bankSize; curPos++)
	{
		if (!memcmp(&romData[curPos], songTitle, 4))
		{
			ptrOffset = curPos + bankAmt;
			printf("Song table list:  0x%04X!\n", headerOffset);
			break;
		}
	}
	if (ptrOffset != 0)
	{
		curPos += 4;
		for (k = 0; k < numSongs; k++)
		{
			for (l = 0; l < 20; l++)
			{
				songNames[k][l] = romData[curPos + l];
			}
			songNames[k][20] = '\0';
			curPos += 20;
		}
	}
}

void seqs2txt(unsigned long list[])
{
	sprintf(outfile, "seqs.txt");
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file seqs.txt!\n");
		exit(2);
	}
	else
	{
		unsigned char command[3];
		int seqNum = 0;
		int curPos = 0;
		int nextPos = 0;
		int curRow = 0;
		unsigned char lowNibble = 0;
		unsigned char highNibble = 0;
		int curNote = 0;
		for (seqNum = 0; seqNum < totalSeqs; seqNum++)
		{
			fprintf(txt, "Sequence %i\n===========================\n", seqNum + 1);
			curPos = list[seqNum];
			curRow = 0;
			command[0] = romData[curPos];
			command[1] = romData[curPos + 1];
			command[2] = romData[curPos + 2];
			while (curRow < patRows)
			{
				command[0] = romData[curPos];
				command[1] = romData[curPos + 1];
				command[2] = romData[curPos + 2];
				if (command[0] == 0)
				{
					fprintf(txt, "(Empty row)\n");
					curPos++;
				}
				else if (command[0] > 0 && command[0] < 0x40)
				{
					fprintf(txt, "Pitch slide: %i\n", command[0]);
					curPos++;
				}
				else if (command[0] == 0x40)
				{
					lowNibble = (command[1] >> 4);
					highNibble = (command[1] & 15);
					fprintf(txt, "Change volume: %i\n", lowNibble);
					curPos += 2;
				}
				else if (command[0] > 0x40 && command[0] < 0x80)
				{
					curNote = command[0] - 0x41;
					fprintf(txt, "Note: %s, instrument: %i\n", noteVals[curNote], command[1]);
					curPos += 2;
				}
				else if (command[0] == 0x80)
				{
					lowNibble = (command[1] >> 4);
					highNibble = (command[1] & 15);
					fprintf(txt, "Change speed/ticks: %i\n", lowNibble);
					curPos += 2;
				}
				else if (command[0] > 0x80 && command[0] < 0xC0)
				{
					curNote = command[0] - 0x80;
					fprintf(txt, "High drum note: %s, instrument: %i\n", noteVals[curNote], command[1]);
					curPos += 2;
				}
				else if (command[0] >= 0xC0)
				{
					lowNibble = (command[2] >> 4);
					highNibble = (command[2] & 15);
					if (command[0] == 0xC0)
					{
						curNote = 0;
					}
					else
					{
						curNote = command[0] - 0xC1;
					}
					fprintf(txt, "Note: %s, instrument: %i, change speed/ticks: %i\n", noteVals[curNote], command[1], lowNibble);
					curPos += 3;
				}
				curRow++;
			}
			fprintf(txt, "\n");
		}
	}
	fclose(txt);

}