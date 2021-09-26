/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.imebra;

public class MutableAssociationMessage extends AssociationMessage {
  private transient long swigCPtr;

  protected MutableAssociationMessage(long cPtr, boolean cMemoryOwn) {
    super(imebraJNI.MutableAssociationMessage_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(MutableAssociationMessage obj) {
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
        imebraJNI.delete_MutableAssociationMessage(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public MutableAssociationMessage(String abstractSyntax) {
    this(imebraJNI.new_MutableAssociationMessage__SWIG_0(abstractSyntax), true);
  }

  public MutableAssociationMessage(MutableAssociationMessage source) {
    this(imebraJNI.new_MutableAssociationMessage__SWIG_1(MutableAssociationMessage.getCPtr(source), source), true);
  }

  public void addDataSet(DataSet dataSet) {
    imebraJNI.MutableAssociationMessage_addDataSet(swigCPtr, this, DataSet.getCPtr(dataSet), dataSet);
  }

}
