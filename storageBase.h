#ifndef STORAGE_BASE_H
#define STORAGE_BASE_H

// Define the interface
class storage{
	public: 
		storage(void) {};
		virtual int Read(char *buffer, int index, int length, int offset = 0) = 0;
		virtual int Write(char *buffer, int index, int length, int offset = 0) = 0;
		virtual int Clear(int index) = 0;
};

class circBuff{
	public:
		circBuff(void) {};
		virtual int Put(char *buffer, int length) = 0;
		virtual int Get(char *buffer, int length) = 0;
		virtual int Probe() = 0;
};

class ramStorage: public storage{
	public:
		ramStorage(int sz);
		~ramStorage(void);
		virtual int Read(char *buffer, int index, int length, int offset = 0) ;
		virtual int Write(char *buffer, int index, int length, int offset = 0) ;
		virtual int Clear(int index);
	protected:
		char *b;
		int bSz;
		int sz;
};

class ramCircBuff: public circBuff{
	public:
		ramCircBuff(int sz);
		~ramCircBuff(void);
		virtual int Put(char *buffer, int length);
		virtual int Get(char *buffer, int length);
		virtual int Probe(void);
	protected:
		char *b;
		int bSz;
		int totalSz;
		int currSz;
		int pWrite;
		int pRead;
};


#endif