import subprocess
import sys
import time
import os
import re

class Sort:

	def __init__(self):
		self.indexFile=None
		self.lexiconFile=None
		self.inBuf=[]
		self.outBuf=[]
		self.indPath="/final/index"
		self.indCount=0

	def UnixSort(self):
		print os.getcwd()
		#subprocess.call(['sort -k 1 -o '+os.getcwd()+'/index/tgt.txt '+os.getcwd()+'/index/index.txt'])
		subprocess.call(['sort','-k','1','-o','tgt.txt','index.txt'])

	def openNextInd(self): 
		self.indexFile.close()
		self.indCount+=1
		return open(os.getcwd()+self.indPath+str(self.indCount),'w')
		
	def mergeDup(self):
		self.lexiconFile=open(os.getcwd()+"/final/lexicon",'w')
		self.indexFile=open(os.getcwd()+self.indPath+"0",'w')
		bufSize=0
		i=0
		with open(os.getcwd()+"/tgt.txt") as index:
			dupList=[]
			prev=None
			dupFlag=False
			for line in index:
				#print sys.getsizeof(self.outBuf)
				record=line.split(' ',1)
				bufSize+=sys.getsizeof(record[1])
				if record[0]==prev:
					dupFlag=True
					a=re.split(r',\s*',record[1])[:-1]
					dupList.extend(a)
				else:
					if dupFlag:
						dupList=sorted(dupList, key=compare)						
						dupFlag=False
					#print str(dupList)
					#generate the difference of DocID instead the actual DocID
					prevd=0
					#print(dupList)
					for j in xrange(len(dupList)):
						x=dupList[j]
						y=x.split(' ',1)
						try:
							#print(y[0])
							dupList[j]=str(int(y[0])-prevd)+" "+y[1]
							prevd=int(y[0])
							#print(x.split(' ',1)[0])
							#print("************")
						except:
							x=''
					if prev:
						self.outBuf.append([prev,dupList])
					prev=record[0]
					dupList=[]
					a=re.split(r',\s*',record[1])[:-1]
					dupList.extend(a)
					if bufSize>52428800: #output buffer once beyond 50MB
						for record in self.outBuf:
							self.lexiconFile.write(record[0]+' '+str(self.indCount)+' '+str(i)+' '+str(len(record[1]))+'\n')
							self.indexFile.write(record[0]+' ')
							i+=1
							for a in record[1]:
								self.indexFile.write(a+',')
							self.indexFile.write('\n')
						self.indexFile=self.openNextInd()
						self.outBuf=[]
						bufSize=0
						i=0
		print bufSize
		for record in self.outBuf:
			self.lexiconFile.write(record[0]+' '+str(self.indCount)+' '+str(i)+'\n')
			self.indexFile.write(record[0]+' ')
			i+=1
			for a in record[1]:
				self.indexFile.write(a+',')
			self.indexFile.write('\n')
		self.indexFile.close()
		self.lexiconFile.close()

def compare(p):
	try:
		q=int(p.split(' ',1)[0])
	except:
		q=-1
	return q

if __name__ == '__main__':
    starttime=time.time()
    s=Sort()
    s.UnixSort()
    print "********************************************"
    s.mergeDup()
    endtime=time.time()
    print (endtime-starttime)/60, 'minutes'
