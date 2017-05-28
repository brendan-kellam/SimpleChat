#ifndef FILETRANSFERDATA_H
#define FILETRANSFERDATA_H

#pragma once
#include <fstream>

struct FileTransferData
{
	static const int	buffersize = 8192;	// Size of buffer packet (in bytes)
	std::string			fileName;			// Name of file
	int				    bytesWritten;		// # of bytes written
	std::streampos		fileOffset;			// byte offset 
	std::streampos		fileSize;			// size of file (in bytes)
	int					remainingBytes;	    // # of bytes not yet sent
	std::ifstream		infileStream;		// Input stream
	std::ofstream		outfileStream;		// Output stream
};

#endif