import argparse
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

colors = ['#ff0000', '#ffff00', '#80d41a', '#404893', '#800080']
sns.set_palette(sns.color_palette(colors))

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('input', type=str, help='Input CSV')
    parser.add_argument('output', type=str, nargs='?', help='Output plot')
    parser.add_argument('--log', default=False, action='store_true', help='Set log scale')
    parser.add_argument('--values', default=False, action='store_true', help='Display values')
    args = parser.parse_args()

    df = pd.read_csv(args.input, sep='\t')
    print(df)
    
    if len(df.columns) == 2:
        x, y = df.columns
        ax = sns.barplot(data=df, x=x, y=y)

    if len(df.columns) == 3:
        x, hue, y = df.columns
        ax = sns.barplot(data=df, x=x, hue=hue, y=y)

    if args.log:
        ax.set_yscale('log')

    if args.values:
        for i in ax.containers:
            ax.bar_label(i,)

    if args.output is None:
        plt.show()
    else:
        plt.savefig(args.output)

