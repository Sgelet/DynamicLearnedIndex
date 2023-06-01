import sys
import matplotlib.pyplot as plt

trans = {
    "uCH" : ["Eilice", "blue", '-'],
    "uCQ" : ["OvL", "orange", '--'],
    "CGAL" : ["Eddy", "green", '-.'],
    "GA" : ["GA", "red", ':']
}
def parse_file(file):
    res = {
        "name": file.name,
        "model": "",
        "Nums": [],
        "Inserting": [],
        "Removing": [],
    }
    acc = 0
    mode = ""
    for line in file:
        line = line.rstrip()
        if res["model"] == "":
            res["model"] = line
            print(line)
            continue

        if "Inserting" in line or "Removing" in line:
            mode = line
            acc = 0
            continue

        row = line.split(' ')
        if acc >= len(res["Nums"]):
            res["Nums"].append(int(row[0]))

        if acc >= len(res[mode]):
            res[mode].append(float(row[3]))
        else:
            res[mode][acc] += float(row[3])
        acc += 1

    res["Inserting"] = [x/3. for x in res["Inserting"]]
    res["Removing"] = res["Removing"].sort()
    return res


def make_plot(data):
    plt.tight_layout()
    ls = ['-','--','-.',(0,(3,2,1,2,1,2)),':']
    cl = ["orange","blue","green"]
    i = 0;
    fig, ax = plt.subplots()
    for d in data:
        ax.plot(d["Nums"],d["Inserting"],label = trans[d["model"]][0],c=trans[d["model"]][1],ls=trans[d["model"]][2])
        i+=1

    #ax.legend(loc="upper left")
    ax.set(xlabel="No. of points",ylabel="Time in seconds",title="Update time on truncated normal distribution")
    ax.legend()
    fig.savefig("update_trunc.png", dpi=300.)

def main():
    # Parse
    data = []
    for path in sys.stdin:
        file = open(path[:-1])
        data += [parse_file(file)]

    make_plot(data)


if __name__ == "__main__":
    main()
