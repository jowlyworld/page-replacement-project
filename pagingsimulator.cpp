#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <list>
#include <iostream>
using namespace std;
int dr = 0;
int dw = 0;
int frameSize;
int totalLines;
int hit;
double hitrate, faultrate;
string db;

struct page {
		string addr;
		string vpn;
		int dirty;
		char rw;
	};

string addrparse(string addr)
{   
	string vpn;
	for (int i = 0; i < 5; i++)
	{   
	   vpn += addr[i];
	}
	return vpn;  
}

int isfound(page *mypage, list<page> pt)
{
	list<page>::iterator itr;
	for (itr = pt.begin(); itr != pt.end(); itr++) {
		if (itr->vpn == mypage->vpn)
			return 1;
	}
	return 0;
}

string getaddr(string line)
{
	string addr;
	for (int i = 0; i < 8; i++)
	{   
	   addr += line[i];
	}
	return addr;
}

void lru(page *mypg, list<page> *pt);

void fifo(page *mypg, list<page> *pt);

void segmentedfifo(page *mypg, list<page> *PT, list<page> *BT, int primSize, int scndrySize);

int main (int argc, char* argv[])
{   
	string prcnt;
	int primbuffSize;
	int secbuffSize;
	frameSize = stoi(argv[2]);
	string algo = argv[3];
	string file = argv[1];
	if (algo == "vms")
	 {
		db = argv[5];
		prcnt = argv[4];
		double perCent = (stoi(prcnt) * 0.01) * frameSize;
		if((perCent - (int)perCent) >= 0.5)
			secbuffSize = (int)perCent + 1;
		else
		secbuffSize = (int)perCent;
				
		primbuffSize = frameSize - secbuffSize;
	 }
	else
		db = argv[4];

	fstream pFile;
	list<page> pageTable;
	list<page> buffTable;

	pFile.open(file, ios::in);
	if (pFile.is_open()) 
	{
		string line;
		char prw;
		string paddr;
		string pagenum;

	
		while (getline(pFile, line))
		{
			prw = line[9];
			totalLines += 1;
			paddr = getaddr(line);
			pagenum = addrparse(paddr);
			page *mypage = new page;
			if (prw == 'W')
				mypage->dirty = 1;
			else
				mypage->dirty = 0;
			mypage->addr = paddr;
			mypage->rw = prw;
			mypage->vpn = pagenum;
			if (db == "debug")
				cout << line << " consists of " << "virtual address: " << mypage->addr << " with page number " << mypage->vpn << " and permission " << mypage->rw << endl;
			if ((pageTable.size() < frameSize) && !isfound(mypage, pageTable) && (algo != "vms")) {
				pageTable.push_back(*mypage);
				dr +=1;
				if (db == "debug")
					cout << "--compulsory miss--"<< endl <<"disk reads up to now: " << dr << endl;
			}
			else if (algo == "lru") 
				lru(mypage, &pageTable);
			else if (algo == "fifo")
				fifo(mypage, &pageTable);
			else if (algo == "vms") {
				segmentedfifo(mypage, &pageTable, &buffTable, primbuffSize, secbuffSize);
				}
			else {
				cout << "Error choosing algorithm\n";
				return 1;
				}
		}
		pFile.close();
		
	}
	hitrate = (double)hit / (double)totalLines;
	faultrate = (double)dr / (double)totalLines;
	cout << "total memory frames: " << frameSize <<"\nevents in trace: " << totalLines << "\ntotal hits: "<< hit << 
	" (hit rate: "<< hitrate << ")\ntotal disk reads: " << dr << " (fault rate: "<< faultrate << ")\ntotal disk writes: " << dw << endl;
}

void lru(page *mypg, list<page> *pt)
{
	int found = 0;
	list<page>::iterator itr;
	for (itr = pt->begin(); itr != pt->end(); itr++) {
		if (itr->vpn == mypg->vpn)
		{
			found = 1;
			break;
		}
	}
	if (found == 1)
	{
		if (db == "debug")    
		cout << "--hit--\n";
		string c1 = itr->vpn; //used to debug
		if (itr->dirty == 1)
			mypg->dirty = 1; 
		pt->erase(itr);
		pt->push_back(*mypg);
		hit	+= 1;
		page buff = pt->back();
		string c2 = buff.vpn; //used to debug
		if (db == "debug") {
			if(c1 != c2)
				cout << "--LRU page hit not working properly--" << endl;
			else
				cout << "--LRU page hit properly works--" << endl;
			}
	}
	else {
		string c3 = pt->front().vpn;//used to debug
		if (pt->front().dirty == 1)
			{
				dw += 1;
				if (db == "debug")
					cout << "disk writes up to now: " << dw <<endl;
			}
		pt->pop_front();
	   
		pt->push_back(*mypg);
		dr += 1;
		string c4 = mypg->vpn;//used to debug
		if (db == "debug") {
			cout << "disk reads up to now: " << dr << endl;
			if((c3 != pt->front().vpn) && (pt->back().vpn == c4))
				cout << "--fault--\n"<< "Page " << (pt->back()).vpn << " in table now, replacing Page "<< c3 << endl;
			else
				cout << "--error with LRU page fault implementation--"<< endl;
		}
	}

}

void segmentedfifo(page *mypg, list<page> *PT, list<page> *BT, int primSize, int scndrySize)
{
	int found = 0; 
	list<page>::iterator itrfound;
	list<page>::iterator itr1;
	list<page>::iterator itr2;
	for (itr1 = PT->begin(); itr1 != PT->end(); itr1++) {
		if (itr1->vpn == mypg->vpn){
			found = 1; 
			itrfound = itr1;
		}
	}
	for (itr2 = BT->begin(); itr2 != BT->end(); itr2++) {
		if (itr2->vpn == mypg->vpn) {
			found = 2;
			itrfound = itr2;

		}
	}
	switch(found)
		{
			case 0:
					if (PT->size() < primSize)
						{
							PT->push_back(*mypg);
							dr += 1;
							if(db == "debug")
								cout << "disk reads up to now: " << dr << endl;
						}
					else
					{
							page *buffer = new page;
							if (primSize > 0) {
								buffer->addr = (PT->front()).addr;
								buffer->rw = (PT->front()).rw;
								buffer->vpn = (PT->front()).vpn;
								buffer->dirty = (PT->front()).dirty;
								PT->pop_front();
								PT->push_back(*mypg);
								BT->push_back(*buffer);
								dr += 1;
								if(db == "debug")
									cout << "disk reads up to now: " << dr << endl;
								if (BT->size() > scndrySize)    {
									if ((BT->front()).dirty == 1)
										{
										dw += 1;
										if(db == "debug")
											cout << "disk writes up to now: " << dw << endl;
										}
									BT->pop_front();
									}
							}
							else {
								buffer->addr = mypg->addr;
								buffer->rw = mypg->rw;
								buffer->vpn = mypg->vpn;
								buffer->dirty = mypg->dirty;
								BT->push_back(*buffer);
								dr += 1;
								if(db == "debug")
									cout << "disk reads up to now: " << dr << endl;
								if (BT->size() > scndrySize)    {
									if ((BT->front()).dirty == 1) {
										dw += 1;
										if(db == "debug")
											cout << "disk writes up to now: " << dw << endl;
									}
									BT->pop_front();
									}

							}       
					}
					break;
			case 1:
					hit += 1;
					if (db == "debug")
						cout << "--Primary buffer hit--\n";
					if (mypg->dirty == 1)
						 itrfound->dirty = 1;
					
					break;
			case 2:
					hit += 1;
					if (itrfound->dirty == 1)
						mypg->dirty = 1;
					BT->erase(itrfound);
					BT->push_back(*mypg);

					break;
			default:
					cout << "--error--\n";
					break;
			
		}

}

void fifo(page *mypg, list<page> *pt)
{   
	int found = 0;
	list<page>::iterator itr;
	for (itr = pt->begin(); itr != pt->end(); itr++) {
		if (itr->vpn == mypg->vpn)
		{
			found = 1;
			break;
		}
	}

	if (found == 1)
	{
		hit += 1;
		if (db == "debug")
			cout << "--hit--"<<endl;
		if ((mypg->dirty == 1))
			itr->dirty = 1;
	}
	else
	{
		string c5 = pt->front().vpn;//used to debug
		string c6 = mypg->vpn;//used to debug
		if (pt->front().dirty == 1)
			{
				dw += 1;
			}
		pt->pop_front();
		pt->push_back(*mypg);
		dr += 1;
		if (db == "debug") {
			cout << "disk writes up to now: " << dw << endl;
			cout << "disk reads up to now: " << dr << endl;
			if((c5 != pt->front().vpn) && (pt->back().vpn == c6))
				cout << "--fault--\n"<< "Page " << (pt->back()).vpn << " in table now, replacing Page "<< c5 << endl;
			else
				cout << "--error with FIFO page fault implementation--"<< endl;
		}
	}

	
}
