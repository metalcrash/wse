import os
lexiconFile=open(os.getcwd()+"/final/lexicon",'r')
with open(os.getcwd()+"/final/index") as index:
	i=0
	for line in index:
		record=lexiconFile.readline()
		record=record.split(' ')
		ilist=line.split(' ',1)
		print ilist[0]
		print record[0]
		if ilist[0] != record[0]:
			print i
			print record[1]
			break
		i+=1
print i
lexiconFile.close() 
