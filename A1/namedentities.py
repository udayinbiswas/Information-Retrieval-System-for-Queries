import nltk
from nltk.corpus import stopwords
from nltk.stem import PorterStemmer
from nltk.tokenize import word_tokenize
from os import listdir
from os.path import isfile, join
import sys
from nltk import ne_chunk, pos_tag, word_tokenize
from nltk.tree import Tree



def get_continuous_chunks(text):
	chunked = ne_chunk(pos_tag(word_tokenize(text)))
	prev = None
	continuous_chunk = []
	current_chunk = []
	for i in chunked:
		if type(i) == Tree:
			current_chunk.append(" ".join([token for token, pos in i.leaves()]))
		elif current_chunk:
			named_entity = " ".join(current_chunk)
			if named_entity not in continuous_chunk:
			     continuous_chunk.append(named_entity)
			     current_chunk = []
		else:
			continue
	return continuous_chunk
	
def addNE(inputfile,c,stoppingwords,stemming,collectionpath):
	if collectionpath[-1]=='/':
		file= open(collectionpath+inputfile, 'r')
	else:
		file = open(collectionpath+'/'+inputfile, 'r')
	lines = file.read().split("\n")
	counter = 1
	stop_words = set(stopwords.words('english'))
	x = open("namedentities.txt",'a')
	for sentence in lines:
		if sentence!='\n' and sentence!='':
			words = get_continuous_chunks(sentence)
			if int(stoppingwords)==1:
				words = [w for w in words if not w in stop_words]
			if int(stemming)==1:
				words = [ps.stem(w) for w in words] 
			for w in words:
				x.write(w+":"+str(c)+":"+str(counter)+"\n")
			counter+=1
			


# MAIN method
ps = PorterStemmer()
stop_words = set(stopwords.words('english'))
print("Named entities...")
collectionpath = sys.argv[1]
apply_stopwords = sys.argv[2]
apply_stemming = sys.argv[3]
test = open("namedentities.txt",'w')
test.write("")
test.close()
filenames = open('original_files.txt','r')
files = [x[:-1] for x in filenames.readlines()]
for f,c in zip(files,range(0,len(files))):
	addNE(f,c,apply_stopwords,apply_stemming,collectionpath)


