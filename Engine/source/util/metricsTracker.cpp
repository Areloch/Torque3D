#include "util/metricsTracker.h"
#include "console/console.h"
#include "console/engineAPI.h"

MetricsTracker gMetricsTracker;

MetricsTracker::MetricsTracker() = default;

void MetricsTracker::addMetric(StringTableEntry metricsGroup, const String& metricsVariable, const String& metricsDescription)
{
   for (MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         for (const MetricsEntry::StringPair& j : i.metricsVarEntry)
         {
            if (j.metricsVariable == metricsVariable)
               return;
         }

         MetricsEntry::StringPair newPair;
         newPair.metricsVariable = metricsVariable;
         newPair.metricsDescription = metricsDescription;
         i.metricsVarEntry.push_back(newPair);
         return;
      }
   }

   MetricsEntry newEntry;
   newEntry.metricsGroup = metricsGroup;
   MetricsEntry::StringPair newPair;
   newPair.metricsVariable = metricsVariable;
   newPair.metricsDescription = metricsDescription;
   newEntry.metricsVarEntry.push_back(newPair);
   mMetricsList.push_back(newEntry);
}

void MetricsTracker::removeMetric(StringTableEntry metricsGroup, const String& metricsVariable)
{
   for (MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         for (Vector<MetricsEntry::StringPair>::iterator it = i.metricsVarEntry.begin(); it != i.metricsVarEntry.end(); it++)
         {
            if (it->metricsVariable == metricsVariable)
            {
               i.metricsVarEntry.erase(it);
               return;
            }
         }
      }
   }
}

S32 MetricsTracker::getMetricsCount(StringTableEntry metricsGroup)
{
   for (const MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         return i.metricsVarEntry.size();
      }
   }

   return 0;
}

const char* MetricsTracker::getMetricsVariable(StringTableEntry metricsGroup, const S32& index)
{
   for (const MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         if (index < i.metricsVarEntry.size() && index >= 0)
            return i.metricsVarEntry[index].metricsVariable.c_str();
      }
   }

   return "";
}

const char* MetricsTracker::getMetricsDescription(StringTableEntry metricsGroup, const S32& index)
{
   for (const MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         if(index < i.metricsVarEntry.size() && index >= 0)
            return i.metricsVarEntry[index].metricsDescription.c_str();
      }
   }

   return "";
}

DefineEngineFunction(addMetric, void, (const char* metricsGroup, const char* metricsVariable, const char* metricDescription), ("", "", ""), "()"
   "@brief Adds a new metric to the tracker so it can be easily displayed via a UI or the console\n"
   "@param metricsGroup - The group to which the metric belongs. This is used to organize the metrics in the UI\n"
   "@param metricsVariable - The variable to be tracked as a metric\n"
   "@ingroup Utils")
{
   gMetricsTracker.addMetric(StringTable->insert(metricsGroup), metricsVariable, metricDescription);
}

DefineEngineFunction(removeMetric, void, (const char* metricsGroup, const char* metricsVariable), ("", ""), "()"
   "@brief Removes a metric from the tracker\n"
   "@param metricsGroup - The group to which the metric belongs\n"
   "@param metricsVariable - The variable to be removed from the tracker\n"
   "@ingroup Utils")
{
   gMetricsTracker.removeMetric(StringTable->insert(metricsGroup), metricsVariable);
}

DefineEngineFunction(getMetricsCount, S32, (const char* metricsGroup), (""), "()"
   "@brief Returns the number of metrics in the specified group\n"
   "@param metricsGroup - The group to which the metrics belong\n"
   "@ingroup Utils")
{
   return gMetricsTracker.getMetricsCount(StringTable->insert(metricsGroup));
}


DefineEngineFunction(getMetricsVariable, const char*, (const char* metricsGroup, S32 index), ("", -1), "()"
   "@brief Returns the variable name of the metric at the specified index\n"
   "@param metricsGroup - The group to which the metrics belong\n"
   "@param index - The index of the metric\n"
   "@ingroup Utils")
{
   return gMetricsTracker.getMetricsVariable(StringTable->insert(metricsGroup), index);
}

DefineEngineFunction(getMetricsDescription, const char*, (const char* metricsGroup, S32 index), ("", -1), "()"
   "@brief Returns the description of the metric at the specified index\n"
   "@param metricsGroup - The group to which the metrics belong\n"
   "@param index - The index of the metric\n"
   "@ingroup Utils")
{
   return gMetricsTracker.getMetricsDescription(StringTable->insert(metricsGroup), index);
}
