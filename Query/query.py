tokenMap={}
docList={}

def initData():
	with open(os.getcwd()+"/final/lexicon") as lexicon:
		for line in lexicon:
			record=line.split()
			#token:index file number, line number, containing doc number
			tokenMap[record[0]]=[record[1],record[2],record[3]]
	with open(os.getcwd()+"/final/lexicon") as doclist:
		for line in doclist:
			record=line.split()
			#docID:URL, token number
			docList[record[0]]=[record[1],record[2]]

def query(terms):
	appears=''
	indexLists=''
	for term in terms:
		indexPos=tokenMap[term]
		appears+='|'+indexPos[3]
		indexFile=open(os.getcwd()+"/final/index"+indexPos[1],'r')
		lNum=indexPos[2]
		indexList=None
		for i, line in enumerate(indexFile):
			if i==lNum:
				indexList=line
				break
		if not indexList:
			print "unfound index list"
			return -1
		indexLists+='|'+indexList
	#call C extension
	topK=processor.process(len(terms),indexLists,appears)
	return topK
		
if __name__ == '__main__':
	initData()
	query()
