import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns; sns.set()
sns.set_style("whitegrid")
import matplotlib.ticker as mtick
import matplotlib
import matplotlib.patches as mpatches
matplotlib.ticker._mathdefault = lambda x: '\\mathdefault{%s}'%x
#Get Non-Type-3 Fonts
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42

legends = ["RFM-16",  "RFM-32"]
                               
#Color Palette
#colors = ['#3DB2FF', 'salmon']

colors = ['salmon', 'black']

#Set Font Size
#plt.rc('font', family='serif')
plt.rc('font', size='9.5')

# Read Dataframe
df = pd.read_csv("./rfmsb.txt", delimiter=r'\s+')
#print (df)

# Format for Slowdown (%) ensure 0s stay 0s.
# for i in ['4K','2K','1K']:
#     slowdown_df = df.copy()
#     slowdown_df.loc[slowdown_df[i] == 0, i] = 1
#     slowdown_df[i] = slowdown_df[i]-1
# print (slowdown_df)

# Set position of bar on X axis
#barwidth
barWidth = 0.23

r1 = np.arange(len(df.index))
r2 = [x + barWidth for x in r1]
r3 = [x + barWidth for x in r2]
#r4 = [x + barWidth for x in r3]
# r5 = [x + barWidth for x in r4]
# r6 = [x + barWidth for x in r5]

#Create Plot
fig = plt.figure(figsize=(10,2.1))
ax = plt.gca()
#Bars
#plt.bar(r1, df['BASE'], width = barWidth, color=colors[0], label=legends[0],edgecolor='k')
plt.bar(r1, df['TS/A.RFMSB.T16'], width = barWidth, color=colors[0], label=legends[0],edgecolor='k')
plt.bar(r2, df['TS/A.RFMSB.T32'], width = barWidth, color=colors[1], label=legends[1],edgecolor='k')
# plt.bar(r4, df['id_alpha100'], width = barWidth, color=colors[3],label=legends[3],edgecolor='k')
# plt.bar(r4, df['suite4'], width = barWidth, color=colors[3],label=legends[3],edgecolor='k')
# plt.bar(r5, df['suite5'], width = barWidth, color=colors[4],label=legends[4],edgecolor='k')
# plt.bar(r6, df['suite6'], width = barWidth, color=colors[5],label=legends[5],edgecolor='k')

#Y=1 Black Line
ax.plot([-0.75,len(df.index)+0.25], [1,1],lw=1.5,color='black')

#ax.annotate("MOAT", xy=(0, 0.1), xytext=(0.03, 0.85), xycoords='axes fraction',
#            fontsize=8, ha='center', va='bottom', weight='bold',
#            bbox=dict(boxstyle='square', fc='white', edgecolor='black'))

# Add xticks on the middle of the group bars
plt.xticks([(r+barWidth*2) for r in range(len(df.index))], df['Expts'])
ax.set_xlim(-0.5,len(df.index)+0.05)
plt.xticks(rotation = 45,ha='right')

ax.tick_params(axis='x', which='major', labelsize=8)
for tick in ax.xaxis.get_major_ticks():
    tick_str = tick.label1.get_text()
    if ((tick_str == "LIGRA") or (tick_str == "SPEC") or \
       (tick_str == "PARSEC") or (tick_str == "STREAM") or \
        (tick_str == "OTHERS") or (tick_str == "ALL") or (tick_str == "Gmean") ):
        tick.label1.set_weight('bold')
    if(tick_str == "Gmean"):
        tick.label1.set_text("Gmean32")
    if (tick_str == "0" or tick_str == "0.0"):
        tick.set_visible(False)

#Add Y-axes labels
plt.ylabel('Norm. Performance', size=10)
ax.tick_params(axis='y', which='major', labelsize=10)
ax.tick_params(axis='x', which='major', labelsize=10)

#Set Y format & limts
#ax.yaxis.set_major_formatter(mtick.PercentFormatter(1,0))
ax.set_ylim(0.75,1.01)
ax.tick_params(axis='x', which='major', pad=-1)
#ax.set_yticks([0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.90, 1.0])

ax.set_yticks([0.75, 0.80, 0.85, 0.90, 0.95, 1.0])

#grid
ax.axis('on')
ax.xaxis.grid()

#Legend
leg_handles=[]
for i in range(0,2):
    leg_handles.append(mpatches.Patch(facecolor=colors[i], lw=1, label=legends[i], edgecolor='k'))
plt.legend(handles = leg_handles, bbox_to_anchor=(0.5,1.07),frameon=False,loc='center',ncol=6,prop={'size':9.5})

#Text
#ax.text(35,1.1, 'Gmean', family='serif',size='9.5',weight='bold',
#        ha='center', va='center')


#Figure
plt.tight_layout()
fig.savefig("./rfmsb.pdf",bbox_inches='tight')
plt.show()
