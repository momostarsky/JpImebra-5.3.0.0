/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.imebra;

public class MutableDicomDirEntry extends DicomDirEntry {
  private transient long swigCPtr;

  protected MutableDicomDirEntry(long cPtr, boolean cMemoryOwn) {
    super(imebraJNI.MutableDicomDirEntry_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(MutableDicomDirEntry obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  @SuppressWarnings("deprecation")
  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        imebraJNI.delete_MutableDicomDirEntry(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public MutableDicomDirEntry(MutableDicomDirEntry source) {
    this(imebraJNI.new_MutableDicomDirEntry(MutableDicomDirEntry.getCPtr(source), source), true);
  }

  public MutableDataSet getEntryDataSet() {
    return new MutableDataSet(imebraJNI.MutableDicomDirEntry_getEntryDataSet(swigCPtr, this), true);
  }

  public void setNextEntry(DicomDirEntry nextEntry) {
    imebraJNI.MutableDicomDirEntry_setNextEntry(swigCPtr, this, DicomDirEntry.getCPtr(nextEntry), nextEntry);
  }

  public void setFirstChildEntry(DicomDirEntry firstChildEntry) {
    imebraJNI.MutableDicomDirEntry_setFirstChildEntry(swigCPtr, this, DicomDirEntry.getCPtr(firstChildEntry), firstChildEntry);
  }

  public void setFileParts(StringsList fileParts) {
    imebraJNI.MutableDicomDirEntry_setFileParts(swigCPtr, this, StringsList.getCPtr(fileParts), fileParts);
  }

}
