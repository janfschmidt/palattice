/* === Metadata Class ===
 * supplies arbitrary additional "meta"information for output files
 * as a nicely formatted file header
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */



// add an entry. if label already exists, update entry
template <class T>
void pal::Metadata::add(string inLabel, T inEntry)
{
  //get string from T
  std::stringstream sEntry;
  sEntry << inEntry;
  //change existing entry
  for(unsigned int i=0; i<label.size(); i++) {
    if (label[i] == inLabel) {
      entry[i] = sEntry.str();
      return;
    }
  }
  //add new entry
  label.push_back(inLabel);
  entry.push_back(sEntry.str());
}
