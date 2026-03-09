import csv

with open('phy_regmap.csv','r') as csvfile:
    regmap = []
    csvreader = csv.reader(csvfile)
    for row in csvreader:
        regmap.append({'sys_addr':int(row[0],16), 'region':row[1], 'reg':row[2]})
        
curr_addr=0

for len(regmap)