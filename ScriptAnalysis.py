import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
df= pd.read_csv("ResultsCsv/test_0.csv")
print(df.info())

print(df.describe())
sns.relplot(x="number test", y="value", data=df, kind="line", ci="sd")
plt.show()

