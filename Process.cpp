#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "Process.hpp"
#include <algorithm>

Process::Process(const std::vector<std::string>& argss, bool verbose) :
    verbose(verbose),
    m_name(argss[0]),
    m_pid((pid_t)NULL),
    m_writepipe {-1,-1},
    m_readpipe {-1,-1},
    m_pwrite((FILE*)NULL),
    m_pread((FILE*)NULL)
{
    if (pipe(m_writepipe) < 0 || pipe(m_readpipe) < 0)
    {
	perror("pipe failure");
	throw std::string("Pipe failure");
    }

    if ((m_pid = fork()) < 0)
    {
perror("fork failure");
throw std::string("fork failure");
    } else if ( m_pid == 0 ) {
	//child
	close(PARENT_READ);
	close(PARENT_WRITE);
	dup2(CHILD_WRITE,1); 
	if(dup2(CHILD_WRITE,1)<0)
	{
		perror("error with child write dup2 call");
		throw std::string("error with dup2 CHILD_WRITE")'
	}
	close(CHILD_WRITE);	
	dup2(CHILD_READ,0); 
	if(dup2(CHILD_READ,0)<0)
	{
		perror("error with child read dup2 call");
		throw std::string("error with dup2 CHILD_READ")'
	}
	close(CHILD_READ);
	std::vector<const char*> args;
	std::transform(argss.begin(),argss.end(), std::back_inserter(args), [](std::string s)
		{
			return s.c_str();
		} );
	args.push_back( NULL );
	execvp(args[0], const_cast<char**>(&args[0]));
	perror("error with execvp");
	throw std::string("error with execvp");
    } else {
	// parent
	if (verbose)
	std::cerr << "Parent process error" << std::endl;
	close(CHILD_READ);
	close(CHILD_WRITE);
	m_pread = fdopen(PARENT_READ, "r");
	m_pwrite = fdopen(PARENT_WRITE, "w");
    }
};

Process::~Process()
{
int temp;    
if (verbose)
	std::cerr << "Process " << m_name << ": Entering ~Process()" << std::endl;
fclose(m_pwrite);    
pid_t pid = waitpid(m_pid, &temp, 0);    
fclose(m_pread);
if (verbose)
	std::cerr << "Process " << m_name << ": Leaving ~Process()" << std::endl;
};

void Process::write(const std::string& line)
{
    fputs(line.c_str(), m_pwrite);
    fflush(m_pwrite);
}

std::string Process::read()
{
    std::string line;
    char* mystring = NULL;
    size_t num_bytes;

    getline(&mystring, &num_bytes, m_pread);
    line = mystring;
    return line;
}
