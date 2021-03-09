import csv
cwb_filename = '107000173.csv'

data = []

#C0A880, C0F9A0, C0G640, C0R190, C0X260 List 
dataname= ['C0A880', 'C0F9A0', 'C0G640', 'C0R190', 'C0X260']

#List to contain the column name;
header=[]

#List for containing information about respective dataname
data0 = []
data1 = []
data2 = []
data3 = []
data4 = []
#invdata = [] used to contain invalid row

#Opening a csv file and name it mycsv
with open(cwb_filename) as csvfile: 
   mycsv = csv.reader(csvfile)
   data=list(mycsv)  
   for row in mycsv:
      data.append(row)

header=data[0]
for i in range(len(header)):
  if(header[i]=='station_id'):
    ID=i
  if(header[i]=='HUMD'):
    humd=i
  

#LOOP through the data
for i in range(len(data)):

  #This 'if' block is to find the IDs required and classify them accordingly
  if(data[i][ID] == dataname[0] or data[i][ID]==dataname[1] or
     data[i][ID] == dataname[2]  or data[i][ID]== dataname[3] or 
     data[i][ID] == dataname[4]):
    
    #This 'if' block checks if the HUMD is an invalid input.
    #If it is INVALID, we go to the next iteration else we append the row to
    #the ID related.

      if(data[i][humd]=='-99.000' or data[i][humd]=='-999.000'):
         #invdata.append(data[i]) append invalid row to a list
        continue
      else: 
        if(data[i][ID] == dataname[0]):
          data0.append(data[i])
        if(data[i][ID] == dataname[1]):
          data1.append(data[i])
        if(data[i][ID] == dataname[2]):
          data2.append(data[i])
        if(data[i][ID] == dataname[3]):
          data3.append(data[i])
        if(data[i][ID] == dataname[4]):
          data4.append(data[i])

#List to contain the summation of HUMD
sum=[0,0,0,0,0]

#List to contain the final answer
target_data=[]

for i in range(len(data0)):
  sum[0] = sum[0] + float(data0[i][humd])
for i in range(len(data1)):
  sum[1] = sum[1] + float(data1[i][humd])
for i in range(len(data2)):
  sum[2] = sum[2] + float(data2[i][humd])
for i in range(len(data3)):
  sum[3] = sum[3] + float(data3[i][humd])
for i in range(len(data4)):
  sum[4] = sum[4] + float(data4[i][humd])

'''
for i in range(len(data0)):
  print(data0[i][6])

print('\n')
for i in range(len(data1)):
  print(data1[i][6])

for i in range(len(sum)):
  print(sum[i])
'''

for i in range(len(sum)):
  if(sum[i]==0):
    sum[i]='None' #Checks if the sum[i] is empty. If it is, sum[i] becomes 'None'
  a=[dataname[i],sum[i]] # Make a list to contain the ID and sum
  target_data.append(a)

#print(invdata) used to check the invalid data id
for i in range(len(target_data)):
  print(target_data[i])

