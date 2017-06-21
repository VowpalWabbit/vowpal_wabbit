Wikipara data set
-------------------------------

This is a data set generation scheme we were playing around with 
which allows us to vary the number of classes and the number 
of training examples per class.

We take some number of wikipedia pages and use the page (id) as the class.
We then take some number of paragraphs from the page as training examples
and some other number of paragraphs from the page as test examples.

A setting we found particularly challenging was 10,000 classes with 
3 training examples and 1 test example.
