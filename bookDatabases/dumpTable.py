import sys
import MySQLdb

table=sys.argv[1]

db=MySQLdb.connect(host="localhost",user="ravel",passwd="ravel",db="hpcoders_BIS")
cur=db.cursor()

cur.execute("show columns from "+table)
cols=cur.fetchall()

# write header row
for c in cols:
    if (c[0]=="id$" or c[0]=="value$"):
        continue
    print c[0]+",",
print "value$"
    
cur.execute("select * from "+table)
for r in cur.fetchall():
    for i in range(len(r)):
        if (cols[i][0]=="id$"):
            continue
        if (cols[i][0]=="value$"):
            value=float(r[i])
            continue
        print '"'+r[i].strip()+'",',
    print value


