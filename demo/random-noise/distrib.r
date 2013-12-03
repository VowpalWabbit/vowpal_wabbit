#!/usr/bin/Rscript
# --vanilla
#
# distrib.r:
#   utility to plot distribution/density of a numeric data-set column
#
#   Usage:
#       distrib.r data_file ["optional chart title string"]
#   where data_file contains the numeric vector, a number per line.
#
# -- where to look for R libraries
# .libPaths(c('~/local/lib/R',
#             '/usr/lib/R/library',
#             '/usr/lib/R/site-library'
# ))
suppressPackageStartupMessages(library(ggplot2))

ratio = 1.61803398875
W = 4
H = W / ratio
DPI = 200
FONTSIZE = 9 
MyGray = 'grey50'

title.theme   = element_text(family="FreeSans", face="bold.italic",
                            size=FONTSIZE-2)
x.title.theme = element_text(family="FreeSans", face="bold.italic",
                            size=FONTSIZE-2, vjust=-0.1)
y.title.theme = element_text(family="FreeSans", face="bold.italic",
                           size=FONTSIZE-2, angle=90, vjust=0.2)
x.axis.theme  = element_text(family="FreeSans", face="bold",
                            size=FONTSIZE-2, colour=MyGray)
y.axis.theme  = element_text(family="FreeSans", face="bold",
                            size=FONTSIZE-2, colour=MyGray)
legend.theme  = element_text(family="FreeSans", face="bold.italic",
                            size=FONTSIZE-1, colour="black")

eprintf <- function(...) cat(sprintf(...), sep='', file=stderr())

argv <- commandArgs(trailingOnly = TRUE)

csvfile <- argv[1]
title <- ifelse(! is.na(argv[2]),
                argv[2],
                'vw demo: random expression distribution')
Ys   <- read.csv(csvfile, header=F, col.names='Ys')

d <- data.frame(Ys=Ys)

Y_labels <- function(yrange) {
    the.min <- as.integer(floor(yrange[1]))
    the.max <- as.integer(ceiling(yrange[2] + 1))
    seq(from=the.min, to=the.max, by=1)
}

# geom_histogram(binwidth=.5, alpha=.5, position="identity")
# geom_histogram(fill='#3377ff',
#        binwidth=.01, alpha=.4, stat='density') +
g <- ggplot(data=d, aes(x=Ys)) +
        geom_density(fill='#3377ff', alpha=0.4, lwd=0.2) +
        scale_x_continuous(breaks=Y_labels(range(Ys))) +
        ggtitle(title) +
        xlab(NULL) +
        theme(
            plot.title=title.theme,
            axis.title.y=y.title.theme,
            axis.title.x=x.title.theme,
            axis.text.x=x.axis.theme,
            axis.text.y=y.axis.theme
        )

pngfile <- sprintf("%s.density.png", csvfile)
ggsave(g, file=pngfile, width=W, height=H, dpi=DPI)


