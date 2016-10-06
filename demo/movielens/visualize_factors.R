# Create movie dendrogram based on latent factors

library(reshape)

##########################################################
# Utility functions
##########################################################

# Load factors file (as preprocessed by shell script) and join with movie names
# for better visualization
loadMovieFactors <- function(name) {
  lrq_movies <- read.csv('lrqdropout.results.model.csv',header=F,sep=':')
  names(lrq_movies) <- c('lrq','namespace','movie','factor','hash','weight')
  movie_list <- read.csv('ml-1m/movies.dat',header=F,sep=':',fileEncoding='latin1')
  movies <- data.frame(movie=movie_list$V1,name=movie_list$V3,genre=movie_list$V5)
  movies$full_name <- paste(movies$name,movies$genre,sep=' / ')
  mm <- merge(lrq_movies,movies,by='movie')
  movie_factors <- cast(mm, full_name ~ factor, value='weight', sum)
  rownames(movie_factors) <- movie_factors$full_name
  subset(movie_factors, select = -c(full_name))
}

# Calculate distances between movie latent factors using cosine similarity
cosineSimilarity <- function(df){
  x <- as.matrix(df)
  m <- 1 - x%*%t(x)/(sqrt(rowSums(x^2) %*% t(rowSums(x^2))))
  rownames(m) <- rownames(df)
  colnames(m) <- rownames(df)
  m
}

# Generate a hierarchical clustering dendrogram, color the N cluster founds with alternating colors
# The movie titles are going to be really small because of the number of movies, better to use
# a dendrogram viewing tool.
clusterMovies <- function(distances, N) {
  hc <- hclust(distances)
  clusMember <- cutree(hc,N)
  labelColors <- rep(c("#036564", "#EB6841"), N/2)
  colLab <- function(n) {
    if (is.leaf(n)) {
      a <- attributes(n)
      labCol <- labelColors[clusMember[which(names(clusMember) == a$label)]]
      attr(n, "nodePar") <- c(a$nodePar, lab.col = labCol)
    }
    n
  }
  hcd <- as.dendrogram(hc)
  dendrapply(hcd, colLab)
}

# Save the dendrogram in PDF so that it is possible to zoom into the movie names
saveDendrogram <- function(dendrogram,fname,w=40,h=15,cex=1.0) {
  pdf(fname, width=w, height=h)
  par(cex=cex,mai=c(8,2,2,2))
  plot(dendrogram)
  dev.off()  
}

movie_factors <- loadMovieFactors('lrqdropout.results.model.csv')
distances <- cosineSimilarity(movie_factors)
dendrogram <- clusterMovies(as.dist(distances), 80)
saveDendrogram(dendrogram,"movie_dendrogram.pdf",w=400,h=150,cex=0.5)
saveDendrogram(cut(dendrogram,h=1.5)$lower[[1]],"movie_dendrogram_small.pdf",w=20)
