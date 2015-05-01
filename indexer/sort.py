import subprocess
import sys
import time
import os
import re
import simple9
import gc

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
		subprocess.call(['sort','-f','-d','-k','1,1','-o','tgt.txt','index.txt'])

	def openNextInd(self): 
		self.indexFile.close()
		self.indCount+=1
		return open(os.getcwd()+self.indPath+str(self.indCount),'w')
		
	def mergeDup(self):
		self.lexiconFile=open(os.getcwd()+"/final/lexicon",'w')
		self.lexiconFile.close()
		index_file_pattern = re.compile(r'^index[0-9]+')
		for root, dirs, files in os.walk(os.getcwd()+"/final"):
			for f in files:
				if index_file_pattern.match(f):
					indexFile=open(os.path.join(root,f),'w')
					indexFile.close()
		logFile=open(os.getcwd()+"log.txt",'w')
		logFile.close()
		bufSize=0
		i=0
		k=0
		with open(os.getcwd()+"/tgt.txt") as index:
			dupList=[]
			prev=None
			dupFlag=False
			for line in index:
				#print sys.getsizeof(self.outBuf)
				record=line.split(' ',1)
				bufSize+=sys.getsizeof(record[1])
				if not record[0].isalnum():
					continue
				if record[0].lower()==prev:
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
					tmp=[]
					for j in xrange(len(dupList)):
						x=dupList[j]
						y=x.split(' ',1)
						try:
							#print(y[0])
							tmp.append(str(int(y[0])-prevd)+" "+y[1])
							prevd=int(y[0])
							#print(x.split(' ',1)[0])
							#print("************")
						except:
							x=''#
					dupList=tmp
					if prev:
						self.outBuf.append([prev,dupList])
					prev=record[0].lower()
					dupList=[]
					a=re.split(r',\s*',record[1])[:-1]
					dupList.extend(a)
					if bufSize>52428800: #output buffer once beyond 50MB
						simple9.encode(self.outBuf,self.indCount)
						'''for record in self.outBuf:
							self.lexiconFile.write(record[0]+' '+str(self.indCount)+' '+str(i)+' '+str(len(record[1]))+'\n')
							#self.indexFile.write(record[0]+' ')
							i+=1
							#print(record[1])
							#print(record[0])
							#print(type(record[1]))
							#print(self.indCount)
							simple9.encode(record[1],self.indCount)
							for a in record[1]:
								print a
								print type(a)
								s=simple9.encode(str(a))
								self.indexFile.write(s+',')
							self.indexFile.write('\n')'''
						self.indCount+=1
						self.outBuf=[]
						bufSize=0
						i=0
		print bufSize
		simple9.encode(self.outBuf,self.indCount)
		'''for record in self.outBuf:
			self.lexiconFile.write(record[0]+' '+str(self.indCount)+' '+str(i)+'\n')
			#self.indexFile.write(record[0]+' ')
			i+=1
			simple9.encode(record[1],self.indCount)
			for a in record[1]:
				#print a
				s=simple9.encode(str(a))
				self.indexFile.write(a+',')
			self.indexFile.write('\n')'''
		#self.indexFile.close()
		#self.lexiconFile.close()

def compare(p):
	try:
		q=int(p.split(' ',1)[0])
	except:
		q=-1
	return q

if __name__ == '__main__':
    starttime=time.time()
    s=Sort()
    #s.UnixSort()
    print "********************************************"
    s.mergeDup()
    endtime=time.time()
    print (endtime-starttime)/60, 'minutes'
