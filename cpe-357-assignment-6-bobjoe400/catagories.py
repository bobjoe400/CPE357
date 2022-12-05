f = open("small.csv", "r")
columns = f.readline().split(",")
cats = {}
i = 0
for column in columns:
    print(str(i)+","+column + ",")
    cat = column.split(".")
    if cat[0] not in cats.keys():
        cats[cat[0]] = []
    if len(cat) > 1:
        cats[cat[0]].append(cat[1])
    i+=1
    
