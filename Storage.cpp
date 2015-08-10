
#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "Storage.cpp"
#endif
#include "MyDebug.h"

#include "Storage.h"

Storage *Storage::storage;

Storage::Storage(uint8_t nRepos): sd(p5,p6,p7,p8,"sd"){
	storage = this;
	nReps = nRepos;
	cnReps = 0;
	repos = (StorageEntry *) malloc(sizeof(StorageEntry)*nReps);
	DBG("Instanciate %d repos", nRepos);
}

Storage::~Storage() {
	for(uint8_t i = 0; i<cnReps; i++)
		FreeRepository(repos+i);
	free(repos);
}

StorageEntry* Storage::InstanciateRepository(StorageEntry *repo) {
	repo->mutex = new Mutex();
	repo->opened = false;
	return repo;
}

void Storage::FreeRepository(StorageEntry *repo) {
	if(repo->opened)
		CloseRepository(repo);
	delete(repo->mutex);
}

StorageEntry *Storage::SearchRepository(const char* repName) {
	DBG("Search repos %s", repName);
	for(uint8_t i = 0; i<cnReps; i++) {
		DBG("%s ?? %s", repName, (repos+i)->name);
		if(!strcmp(repName,(repos+i)->name)) {
			DBG("Match");
			return repos+i;	
		}
	}
	return NULL;
}

void Storage::OpenRepository(const char* repName, StorageEntry *repo) {
	//DBG("Open repo");
	repo->mutex->lock();
	if(!repo->opened) {
		//DBG("Not open open it");
		char path[32];
		if(strlen(repName)<15) {
			strcpy(repo->name,repName);
			sprintf(path,"/sd/%s.txt",repo->name);
			repo->fp = fopen(path, "a+");
			repo->opened = true;
			//DBG("Opened repo %d", nReps);
		} else {
			ERR("Repo %s could not be opened/created because the file name is to long");
		}
	}
	repo->mutex->unlock();
}

void Storage::CloseRepository(StorageEntry *repo) {
	//DBG("Close repo");
	repo->mutex->lock();
	if(repo->opened) {
		//DBG("Opened Close it");
		//strcpy(repo->name,"none");
		fclose(repo->fp);
		repo->opened = false;
		//DBG("Closed repo %s", nReps);
	}
	repo->mutex->unlock();
}

uint16_t Storage::WriteData(const char* repo, uint8_t *buff, uint16_t n, bool overwrite) {
	StorageEntry *se = SearchRepository(repo);
	if(se == NULL) {
		if(cnReps<nReps){
			DBG("Write: Open new file");
			se = InstanciateRepository(repos+cnReps);
			OpenRepository(repo,se);
			cnReps++;
		} else {
			ERR("Could not Instanciate new repos, max reps reached %d", nReps);
			return 0;
		}
	}
	se->mutex->lock();
	if(overwrite) {
		DBG("Need to overwrite Data");
		char p1[32];
		char p2[32];
		sprintf(p1,"/sd/%s.txt",se->name);
		sprintf(p2,"/sd/%snew.txt",se->name);
		FILE *cpyfp = fopen(p2, "w+");
		DBG("Close Old");
		fclose(se->fp);
		fclose(cpyfp);
		DBG("Delete Old");
		int res = remove(p1);
		if(res == -1)
			DBG("ERROR removing file");
		rename(p2,p1);
		cpyfp = fopen(p1, "a+");
		se->fp = cpyfp;
	}
	//DBG("fseek");
	fseek(se->fp,0,SEEK_END);
	//DBG("fwrite");
	int r = fwrite(buff,1,n,se->fp);
	se->mutex->unlock();
	return r;
}

uint16_t Storage::ReadDataSize(const char* repo) {
	StorageEntry *se= SearchRepository(repo);
	int s = 0;
	if(se != NULL) {
		se->mutex->lock();
		fseek(se->fp,0,SEEK_END);
		s = ftell(se->fp);
		fseek(se->fp,0,SEEK_SET);
		se->mutex->unlock();
	} else {
		ERR("No Repo %s to read data size", repo);
	}
	return s;
}

uint16_t Storage::ReadData(const char* repo, uint8_t *buff, uint16_t max, int16_t offset, bool erase) {
	StorageEntry *se= SearchRepository(repo);
	if(se == NULL) {
		if(cnReps<nReps){
			DBG("Read: Open new file");
			se = InstanciateRepository(repos+cnReps);
			OpenRepository(repo,se);
			cnReps++;
		} else {
			ERR("Could not Instanciate new repos, max reps reached %d", nReps);
			return 0;
		}
	}
	se->mutex->lock();
	fseek(se->fp,0,SEEK_END);
	//DBG("fseek");
	int s = ftell(se->fp);
	fseek(se->fp,0,SEEK_SET);
	//DBG("fread");
	int r = fread(buff,1,max,se->fp);
	// This is increadibly expensive in execution term... 
	// I should find something more smart to do the cpy.
	if(erase) {
		DBG("Create New File");
		char p1[32];
		char p2[32];
		sprintf(p1,"/sd/%s.txt",se->name);
		sprintf(p2,"/sd/%snew.txt",se->name);
		char temp[512];
		FILE *cpyfp = fopen(p2, "w+");
		DBG("Copy from %d to %d (%d bytes)", r, s, s-r);
		while(1) {
			int cpsz = fread(temp,1,512,se->fp);
			DBG("Copying %d chars", cpsz);
			if(cpsz) {
				DBG("Copy?");
				fwrite(temp,1,cpsz,cpyfp);
			} else {
				DBG("Reached EOF");
				break;
			}
		}
		DBG("Close Old");
		fclose(se->fp);
		fclose(cpyfp);
		DBG("Delete Old");
		int res = remove(p1);
		if(res == -1)
			ERR("ERROR removing file");
		rename(p2,p1);
		cpyfp = fopen(p1, "a+");
		se->fp = cpyfp;
	}
	se->mutex->unlock();
	return r;
}

void Storage::daRemove(const char* file) {
	remove(file);
}



