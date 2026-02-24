/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUploadHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 00:04:33 by pdrettas          #+#    #+#             */
/*   Updated: 2026/02/23 03:30:02 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <unistd.h>
#include <limits.h>

// helper ft 1
std::string extractBoundary(std::string &contentType)
{
    HttpResponce resp;
    
    size_t bpos = contentType.find("boundary=");
    if (bpos == std::string::npos)
    {
        resp.setStatus(400, "Bad Request");
        resp.setBody("No boundary found");
        return "";
    }

    std::string boundary = contentType.substr(bpos + 9); // "boundary=" will not be taken, only everything that comes after that is saved into boundary string
    size_t semi = boundary.find(";"); // sometimes: check if ; is in boundary signiyfing the end (only until ; is extracted)
    if (semi != std::string::npos)
        boundary = boundary.substr(0, semi);

    std::string delimiter = "--" + boundary; // -- requirement of mmultipart body format to have 

    return delimiter;
}

// helper ft 2
int getDataStart(std::string &body, std::string &delimiter, std::string &filename)
{
    HttpResponce resp;
    
    // find start of first part
    size_t partStart = body.find(delimiter);
    if (partStart == std::string::npos) // if boundary is not in body 
    {
        resp.setStatus(400, "Bad Request");
        resp.setBody("Invalid multipart format");
        return -1;
    }

    partStart += delimiter.length() + 2; // skip \r\n // partStart begins at Content-Disposition

    // extract filename
    size_t filenamePos = body.find("filename=\"", partStart);
    if (filenamePos == std::string::npos)
    {
        resp.setStatus(400, "Bad Request");
        resp.setBody("No filename found");
        return -1;
    }

    filenamePos += 10; // to go to end of "filename=""
    size_t filenameEnd = body.find("\"", filenamePos); // looks for " at end of the name of file
    filename = body.substr(filenamePos, filenameEnd - filenamePos); // takes name of file

    // keep uploaded files inside directory, nowhere else saved outside the folder
    if (filename.find("..") != std::string::npos) // ../
    {
        resp.setStatus(403, "Forbidden");
        resp.setBody("Invalid filename");
        return -1;
    }

    // find file data start
    size_t dataStart = body.find("\r\n\r\n", filenameEnd); // looks for rnrn starting at filenameend. dataStart starts at rnrn
    if (dataStart == std::string::npos) // if \r\n\r\n not found
    {
        resp.setStatus(400, "Bad Request");
        resp.setBody("Invalid file format");
        return -1;
    }
    dataStart += 4; // skips the rnrn to start at actual content of file

    return dataStart;
}

// helper ft 3
int getDataEnd(std::string &body, std::string &delimiter, int dataStart)
{
    HttpResponce resp;
    
    size_t dataEnd = body.find(delimiter, dataStart); // gives me position of end (goes thru from start until delimiter (boundary))
    if (dataEnd == std::string::npos) // if no beginning of boundary found
    {
        resp.setStatus(400, "Bad Request");
        resp.setBody("Invalid multipart ending");
        return -1;
    }

    dataEnd -= 2; // remove \r\n at end (now its at actual end of content of file)
    return dataEnd;
}

// helper ft 4
std::string getFilePath(std::string &filename)
{
    HttpResponce resp;
     
    if (chdir("uploads") == -1) // goes from zproject into upload folder
    {
        resp.setStatus(500, "Internal Server Error");
        resp.setBody("Cannot access uploads directory");
        return "";
    }

    std::string path = filename; // build relative path consisting of just the filename now
    
    return path;
}

// helper ft 5
void createFileAndWriteContent(std::string &path, std::string &fileContent)
{
    HttpResponce resp;
    
    std::ofstream out(path.c_str(), std::ios::binary); // actual file is being created by path into ofstream
    if (!out.is_open()) // if rohr is closed
    {
        resp.setStatus(500, "Internal Server Error");
        resp.setBody("Failed to save file");
        return;
    }

    out.write(fileContent.c_str(), fileContent.size()); // content of file is written into rohr
    out.close();

    resp.setStatus(201, "Created");
    resp.setBody("<html><body><h1>Upload Successful!</h1></body></html>");
    resp.setHeader("Content-Type", "text/html");
    std::cout << "Saving file tto: " << path << std::endl;
    std::cout << "File size: " << fileContent.size() << std::endl;
}

// called in POST request if client needs to upload a file
/*
--boundary123\r\n
Content-Disposition: form-data; name="file"; filename="hello.txt"\r\n
Content-Type: text/plain\r\n
\r\n
<actual file bytes here>
\r\n
--boundary123--\r\n
*/
// 1. extract boundary from Content-Type header (--boundary123\r\n) [boundary changes at every request]
// 2. use boundary to find & extract file content from body
// 3. write content to disk (disk: saving file to the filesystem)
// first part of it is parsing. rest is writing file content to disk
int handleHttpFileUpload(std::string &contentType, std::string &requestBody)
{
    std::string delimiter = extractBoundary(contentType); // TODO: change delimiter name to boundary ??
    if (delimiter == "")
        return -1;
        
    std::string &body = requestBody;
    std::string filename;
    int dataStart = getDataStart(body, delimiter, filename);
    if (dataStart == -1)
        return -1;
        
    int dataEnd = getDataEnd(body, delimiter, dataStart);
    if (dataEnd == -1)
        return -1;
        
    std::string fileContent = body.substr(dataStart, dataEnd - dataStart); // only get actual content (so without content-type line)
    std::cout << "fileContent = " << fileContent << std::endl; // TODO: delete later
    
    std::string path = getFilePath(filename);
    if (path == "")
        return -1;
        
    createFileAndWriteContent(path, fileContent);

    return 0;
}
