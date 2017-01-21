#!/usr/bin/Rscript
# --vanilla
#
# Utility to plot two data-set numeric columns against each other,
# and generate a X-vs-Y chart + pearson correlation between the two.
#
# Usage: x-vs-y.r data_file1 data_file2 pngfile
#   Where:
#       data_file1 & data_file2
#           each contains one data column from some data-set.
#           Columns can be easily pre-extracted into the files using
#                cut -d... -f... dataset
#
#       pngfile
#           is the output chart
#

# -- where to look for R libraries
# .libPaths(c('~/local/lib/R',
#             '/usr/lib/R/library',
#             '/usr/lib/R/site-library'
# ))
suppressPackageStartupMessages(library(ggplot2))

one_column <- function(filename, sep, fieldno) {
    cmd <- sprintf("cut -d '%s' -f %d '%s'", sep, fieldno, filename)
    read.table(pipe(cmd), header=F)[[1]]
}

ratio = 1
W = 6
H = W / ratio
DPI = 200
FONTSIZE = 12 
MyGray = 'grey50'

title.theme   = element_text(family="FreeSans", face="bold.italic",
                            size=FONTSIZE)
x.title.theme = element_text(family="FreeSans", face="bold.italic",
                            size=FONTSIZE, vjust=-0.1)
y.title.theme = element_text(family="FreeSans", face="bold.italic",
                           size=FONTSIZE, angle=90, vjust=0.2)
x.axis.theme  = element_text(family="FreeSans", face="bold",
                            size=FONTSIZE-2, colour=MyGray)
y.axis.theme  = element_text(family="FreeSans", face="bold",
                            size=FONTSIZE-2, colour=MyGray)
legend.theme  = element_text(family="FreeSans", face="bold.italic",
                            size=FONTSIZE-1, colour="black")

eprintf <- function(...) cat(sprintf(...), sep='', file=stderr())

argv <- commandArgs(trailingOnly = TRUE)

# Xs <- read.csv(argv[1], header=F, col.names='X', colClasses=c('numeric'))
Xs <- as.numeric(one_column(argv[1], sep=' ', 1))
# Ys <- read.csv(argv[2], header=F, col.names='Y', colClasses=c('numeric'))
Ys <- as.numeric(one_column(argv[2], sep=' ', 1))

pearson <- as.numeric(cor(Xs, Ys))
eprintf("\nPearson Correlation: %.12f\n", pearson)

d <- data.frame(X=Xs, Y=Ys)

title <-
    sprintf('vw demo: expected vs actual values\nPearson correlation: %.12f',
             pearson)

adaptive.alpha <- 0.02 + (abs(2.0 - (2.0 * pearson))) * 0.3
# eprintf("adaptive.alpha=%g\n", adaptive.alpha)

g <- ggplot(data=d, aes(x=X, y=Y), ) +
        geom_point(shape=20, alpha=adaptive.alpha, size=0.4) +
        ggtitle(title) +
        theme(
            plot.title=title.theme,
            axis.title.y=y.title.theme,
            axis.title.x=x.title.theme,
            axis.text.x=x.axis.theme,
            axis.text.y=y.axis.theme
        )

pngfile <- ifelse(exists(argv[3]) && nchar(argv[3]) > 0, argv[3], 'X-vs-Y.png')
# eprintf("ggsave: pngfile=%s\n", pngfile)
ggsave(g, file=pngfile, width=W, height=H, dpi=DPI)


