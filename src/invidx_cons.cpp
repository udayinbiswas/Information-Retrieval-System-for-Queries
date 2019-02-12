#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include <iterator>
#include <sstream>
#include <tuple>
#include <cmath>

using namespace std;


template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

int main(int argc, char *argv[]){
	string stemming = "0" ;
	string stopword = "0" ;
	bool collpathmen = false;
	string docpath = "" ;
	string indexfile = "";
	for (int i=1;i<argc;i++){
		string argument = argv[i];
		if (argument=="--stemming=0"){
			stemming = "0";
		} else if (argument=="--stemming=1"){
			stemming = "1";
		} else if (argument=="--stopwordremoval=0"){
			stopword = "0";
		} else if (argument=="--stopwordremoval=1"){
			stopword = "1";
		} else if (!collpathmen){
			docpath = argument;
			collpathmen = true;
		} else {
			indexfile = argument;
		}
	}
	
	cout<<stemming<<","<<stopword<<","<<docpath<<","<<indexfile<<endl;
	string preprocess = "python preprocess.py "+docpath+" "+stopword+" "+stemming;
	int res = system(preprocess.c_str());
	preprocess = "python namedentities.py "+docpath+" "+stopword+" "+stemming;
	res = system(preprocess.c_str());
	//Now make a dictionary of the preprocessed files
	ifstream infile("pp_files.txt");
	vector<string> doc;
	vector<int> docid;
	string filename;
	int counter = 0;
	while (infile>>filename){
		doc.push_back(filename);
		docid.push_back(counter);
		counter++;
	}
	infile.close();
	// Parse over each file to create a map of word and its corresponding documents 
	map<string,set< tuple <int,int> > > mydict;
	map<string,set<int> > docfreq;
	map<int,map<string,map<int,int> > > normevaluate;
	int numDocs=doc.size();
	for (int i=0;i<doc.size();i++){
		ifstream infile("preprocessed/"+doc[i]);
		string line;
		int count = 0;
		while (getline(infile,line)){
			count++;
			vector<std::string> x = split(line, ' ');
			for (int j=0;j<x.size();j++){
				string word = x[j];
				tuple<int,int> location = make_tuple(docid[i],count);
				mydict[word].insert(location);
				docfreq[word].insert(docid[i]);
				normevaluate[docid[i]][word][count] += 1;
			}
		}
		infile.close();
	}
	// Parse over named entities
	map<string,set<tuple<int,int> > > mynedict;
	map<string,set<int> > nefreq;
	map<int,map<string,map<int,int> > > nenormevaluate;
    ifstream namedentities("namedentities.txt");
    string fn;
    string did;
    string para;
    string line;
    while (getline(namedentities,line)){
    	size_t found1 = line.find_first_of(":");
		size_t found2 = line.find_last_of(":");
		string fn =  line.substr(0,found1) ;
		string did = line.substr(found1+1,found2);
		string para = line.substr(found2+1);
    	mynedict[fn].insert(make_tuple(stoi(did),stoi(para)));
    	nefreq[fn].insert(stoi(did));
    	nenormevaluate[stoi(did)][fn][stoi(para)] +=1;
    }
    namedentities.close();

	// Create dictionary file
	string dictionary = indexfile +".dict";
	ofstream outfile(dictionary);
	counter= 0;
	outfile<<stopword<<" "<<stemming<<"\n";
    for (map<string,set<tuple<int,int> > >::iterator i=mydict.begin(); i!=mydict.end(); ++i){
    	outfile<<i->first<<":"<<docfreq[i->first].size()<<":"<<counter<<"\n";
    	counter++;
    }
    cout<<counter<<endl;
    outfile<<"NE\n";
    for (map<string,set<tuple<int,int> > >::iterator i=mynedict.begin(); i!=mynedict.end(); ++i){
    	outfile<<i->first<<":"<<nefreq[i->first].size()<<":"<<counter<<"\n";
    	counter++;
    }
    outfile.close();
    cout<<"Dictionary created."<<endl;

    // Create postings list file
    string postlist = indexfile+".idx";
    ofstream outfile2(postlist,ofstream::binary| ios::out);

    for (map<string,set<tuple<int,int> > >::iterator i=mydict.begin(); i!=mydict.end(); ++i){
    	for (set<tuple<int,int> >::iterator j=i->second.begin();j!=i->second.end();++j){
    		outfile2<< get<0>(*j) <<","<< get<1>(*j) <<" ";
    	}
    	outfile2<<"\n";
    }
	for (map<string,set<tuple<int,int> > >::iterator i=mynedict.begin(); i!=mynedict.end(); ++i){
		for (set<tuple<int,int> >::iterator j=i->second.begin();j!=i->second.end();++j){
			outfile2<< get<0>(*j) <<","<< get<1>(*j) <<" ";
		}
		outfile2<<"\n";
	}
    outfile2.close();
    cout<<"Postings list file created"<<endl;


    // Get the Norms
    cout<<"Number of documents:"<<numDocs<<endl;
    map<int,double> norms;
	string normlist = "norms.txt";
	string tflist = "termfrequencies.txt";
	string netflist = "netermfrequencies.txt";
    ofstream outfile3(normlist,ofstream::binary| ios::out);
    ofstream outfile4(tflist,ofstream::binary| ios::out);
    for (map<int,map<string,map<int,int> > >::iterator i= normevaluate.begin();i!=normevaluate.end();i++){
    	int docid = i->first;
    	double allscores=0;
    	map<string,map<int,int> > wordfreq = i->second;
    	for (map<string,map<int,int> >::iterator j=wordfreq.begin();j!=wordfreq.end();j++){
    		string word = j->first;
    		int df = docfreq[word].size();
    		double score = 0;
    		for (map<int,int>::iterator k= j->second.begin();k!=j->second.end();k++){
    			int para = k->first;
    			int tfinpara = k->second;
    			double tf = (1+(log(tfinpara)/log(2)))*(1/pow(2,double(para)));
    			score+= tf*(log(1+double(numDocs)/df)/log(2));
    		}
    		
    		outfile4<<docid<<" "<<word<<" "<<score<<"\n";
    		allscores+= pow(score,2);
    	}
    	double norm = sqrt(allscores);
    	norms[docid] = norm;
    	outfile3<<docid<<" "<<norm<<"\n";
    }
    outfile3.close();
    outfile4.close();
    ofstream outfile5(netflist,ofstream::binary| ios::out);
    for (map<int,map<string,map<int,int> > >::iterator i= nenormevaluate.begin();i!=nenormevaluate.end();i++){
    	int docid = i->first;
    	map<string,map<int,int> > wordfreq = i->second;
    	for (map<string,map<int,int> >::iterator j=wordfreq.begin();j!=wordfreq.end();j++){
    		string word = j->first;
    		int df = nefreq[word].size();
    		double score = 0;
    		for (map<int,int>::iterator k= j->second.begin();k!=j->second.end();k++){
    			int para = k->first;
    			int tfinpara = k->second;
    			double tf = (1+(log(tfinpara)/log(2)))*(1/pow(2,double(para)));
    			score+= tf*(log(1+double(numDocs)/df)/log(2));
    		}
    		
    		outfile5<<docid<<":"<<word<<":"<<score<<"\n";
    	}
    }
    outfile5.close();
    cout<<"Norms of documents evaluated"<<endl;
	return 0;
}