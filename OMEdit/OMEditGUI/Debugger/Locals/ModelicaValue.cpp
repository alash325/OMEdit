/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S ACCEPTANCE
 * OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3, ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 *
 * @author Adeel Asghar <adeel.asghar@liu.se>
 *
 *
 */

#include "ModelicaValue.h"
#include "LocalsWidget.h"
#include "CommandFactory.h"

ModelicaValue::ModelicaValue(LocalsTreeItem *pLocalsTreeItem)
  : QObject(pLocalsTreeItem)
{
  mpLocalsTreeItem = pLocalsTreeItem;
  mValue = "";
}

ModelicaCoreValue::ModelicaCoreValue(LocalsTreeItem *pLocalsTreeItem)
  : ModelicaValue(pLocalsTreeItem)
{

}

QString ModelicaCoreValue::getValueString()
{
  /* if the variable type is modelica_boolean */
  if (mpLocalsTreeItem->getType().compare(Helper::MODELICA_BOOLEAN) == 0)
  {
    QString result = getValue().mid(0, getValue().indexOf(" "));
    if (result.compare("1") == 0)
      return "true";
    else if (result.compare("0") == 0)
      return "false";
    else
      return getValue();
  }
  return getValue();
}

ModelicaRecordValue::ModelicaRecordValue(LocalsTreeItem *pLocalsTreeItem)
  : ModelicaValue(pLocalsTreeItem)
{
  mRecordElements = 0;
}

void ModelicaRecordValue::retrieveChildrenSize()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  pGDBAdapter->postCommand(CommandFactory::arrayLength(mpLocalsTreeItem->getName()), this, &GDBAdapter::arrayLengthCB);
}

QString ModelicaRecordValue::getValueString()
{
  return getValue();
}

void ModelicaRecordValue::setChildrenSize(QString size)
{
  setRecordElements(size.toInt());
  /* invalidate the view so that the items show the updated values. */
  mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getLocalsTreeProxyModel()->invalidate();
}

void ModelicaRecordValue::retrieveChildren()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  for (int i = 2 ; i <= getRecordElements() ; i++)
  {
    QByteArray cmd = CommandFactory::getMetaTypeElement(mpLocalsTreeItem->getName(), i, CommandFactory::record_metaType);
    pGDBAdapter->postCommand(cmd, mpLocalsTreeItem, &GDBAdapter::getMetaTypeElementCB);
  }
}

ModelicaListValue::ModelicaListValue(LocalsTreeItem *pLocalsTreeItem)
  : ModelicaValue(pLocalsTreeItem)
{
  mListLength = 0;
}

void ModelicaListValue::retrieveChildrenSize()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  pGDBAdapter->postCommand(CommandFactory::listLength(mpLocalsTreeItem->getName()), this, &GDBAdapter::arrayLengthCB);
}

QString ModelicaListValue::getValueString()
{
  return QString("<%1 item%2>").arg(getListLength()).arg(getListLength() > 1 ? "s" : "");
}

void ModelicaListValue::setChildrenSize(QString size)
{
  setListLength(size.toInt());
  mpLocalsTreeItem->setDisplayValue(getValueString());
  /* invalidate the view so that the items show the updated values. */
  mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getLocalsTreeProxyModel()->invalidate();
}

void ModelicaListValue::retrieveChildren()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  for (int i = 1 ; i <= getListLength() ; i++)
  {
    QByteArray cmd = CommandFactory::getMetaTypeElement(mpLocalsTreeItem->getName(), i, CommandFactory::list_metaType);
    pGDBAdapter->postCommand(cmd, mpLocalsTreeItem, &GDBAdapter::getMetaTypeElementCB);
  }
}

ModelicaOptionValue::ModelicaOptionValue(LocalsTreeItem *pLocalsTreeItem)
  : ModelicaValue(pLocalsTreeItem)
{
  mIsOptionNone = true;
}

void ModelicaOptionValue::retrieveChildrenSize()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  pGDBAdapter->postCommand(CommandFactory::isOptionNone(mpLocalsTreeItem->getName()), this, &GDBAdapter::arrayLengthCB);
}

QString ModelicaOptionValue::getValueString()
{
  return isOptionNone() ? "NONE()" : "SOME()";
}

void ModelicaOptionValue::setChildrenSize(QString size)
{
  if (size.compare("1") == 0)
    setOptionNone(true);
  else
    setOptionNone(false);
  mpLocalsTreeItem->setDisplayValue(getValueString());
  /* invalidate the view so that the items show the updated values. */
  mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getLocalsTreeProxyModel()->invalidate();
}

void ModelicaOptionValue::retrieveChildren()
{
  if (!isOptionNone())
  {
    GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
    QByteArray cmd = CommandFactory::getMetaTypeElement(mpLocalsTreeItem->getName(), 1, CommandFactory::option_metaType);
    pGDBAdapter->postCommand(cmd, mpLocalsTreeItem, &GDBAdapter::getMetaTypeElementCB);
  }
}

ModelicaTupleValue::ModelicaTupleValue(LocalsTreeItem *pLocalsTreeItem)
  : ModelicaValue(pLocalsTreeItem)
{
  mTupleElements = 0;
}

void ModelicaTupleValue::retrieveChildrenSize()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  pGDBAdapter->postCommand(CommandFactory::arrayLength(mpLocalsTreeItem->getName()), this, &GDBAdapter::arrayLengthCB);
}

QString ModelicaTupleValue::getValueString()
{
  return QString("<%1 item%2>").arg(getTupleElements()).arg(getTupleElements() > 1 ? "s" : "");
}

void ModelicaTupleValue::setChildrenSize(QString size)
{
  setTupleElements(size.toInt());
  mpLocalsTreeItem->setDisplayValue(getValueString());
  /* invalidate the view so that the items show the updated values. */
  mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getLocalsTreeProxyModel()->invalidate();
}

void ModelicaTupleValue::retrieveChildren()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  for (int i = 1 ; i <= getTupleElements() ; i++)
  {
    QByteArray cmd = CommandFactory::getMetaTypeElement(mpLocalsTreeItem->getName(), i, CommandFactory::tuple_metaType);
    pGDBAdapter->postCommand(cmd, mpLocalsTreeItem, &GDBAdapter::getMetaTypeElementCB);
  }
}

MetaModelicaArrayValue::MetaModelicaArrayValue(LocalsTreeItem *pLocalsTreeItem)
  : ModelicaValue(pLocalsTreeItem)
{
  mArrayLength = 0;
}

void MetaModelicaArrayValue::retrieveChildrenSize()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  pGDBAdapter->postCommand(CommandFactory::arrayLength(mpLocalsTreeItem->getName()), this, &GDBAdapter::arrayLengthCB);
}

QString MetaModelicaArrayValue::getValueString()
{
  return QString("<%1 item%2>").arg(getArrayLength()).arg(getArrayLength() > 1 ? "s" : "");
}

void MetaModelicaArrayValue::setChildrenSize(QString size)
{
  setArrayLength(size.toInt());
  mpLocalsTreeItem->setDisplayValue(getValueString());
  /* invalidate the view so that the items show the updated values. */
  mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getLocalsTreeProxyModel()->invalidate();
}

void MetaModelicaArrayValue::retrieveChildren()
{
  GDBAdapter *pGDBAdapter = mpLocalsTreeItem->getLocalsTreeModel()->getLocalsWidget()->getDebuggerMainWindow()->getGDBAdapter();
  for (int i = 1 ; i <= getArrayLength() ; i++)
  {
    QByteArray cmd = CommandFactory::getMetaTypeElement(mpLocalsTreeItem->getName(), i, CommandFactory::array_metaType);
    pGDBAdapter->postCommand(cmd, mpLocalsTreeItem, &GDBAdapter::getMetaTypeElementCB);
  }
}
