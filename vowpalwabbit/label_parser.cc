#include "label_parser.h"
#include "label.h"

void polylabel_copy_label(polylabel& left, polylabel& right)
{
  left = right;
}

void polylabel_delete_label(polylabel& label)
{
  label.reset();
}