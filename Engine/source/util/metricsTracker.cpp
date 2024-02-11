#include "util/metricsTracker.h"
#include "console/console.h"
#include "console/engineAPI.h"

MetricsTracker gMetricsTracker;

MetricsTracker::MetricsTracker() = default;

void MetricsTracker::addMetric(StringTableEntry metricsGroup, const String& metricsVariable, const String& metricsDescription, const bool& graph, const LinearColorF& graphColor, const S32& graphIntervalRate)
{
   for (MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         for (const MetricsVariable& j : i.metricsVarEntry)
         {
            if (j.metricsVariable == metricsVariable)
               return;
         }

         MetricsVariable newVar;
         newVar.metricsVariable = metricsVariable;
         newVar.metricsDescription = metricsDescription;
         newVar.graph = graph;
         newVar.graphColor = graphColor;
         newVar.graphIntervalRate = graphIntervalRate;
         i.metricsVarEntry.push_back(newVar);
         return;
      }
   }

   MetricsEntry newEntry;
   newEntry.metricsGroup = metricsGroup;
   MetricsVariable newVar;
   newVar.metricsVariable = metricsVariable;
   newVar.metricsDescription = metricsDescription;
   newVar.graph = graph;
   newVar.graphColor = graphColor;
   newVar.graphIntervalRate = graphIntervalRate;
   newEntry.metricsVarEntry.push_back(newVar);
   mMetricsList.push_back(newEntry);
}

void MetricsTracker::removeMetric(StringTableEntry metricsGroup, const String& metricsVariable)
{
   for (MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         for (Vector<MetricsVariable>::iterator it = i.metricsVarEntry.begin(); it != i.metricsVarEntry.end(); it++)
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

const char* MetricsTracker::getMetricsGraphInfo(StringTableEntry metricsGroup, const S32& index)
{
   for (const MetricsEntry& i : mMetricsList)
   {
      if (i.metricsGroup == metricsGroup)
      {
         if (index < i.metricsVarEntry.size() && index >= 0)
         {
            static constexpr U32 bufSize = 64;
            char* retBuffer = Con::getReturnBuffer(bufSize);
            dSprintf(retBuffer, bufSize, "%d\t%g %g %g\t%d", i.metricsVarEntry[index].graph,
               i.metricsVarEntry[index].graphColor.red, i.metricsVarEntry[index].graphColor.green, i.metricsVarEntry[index].graphColor.blue,
               i.metricsVarEntry[index].graphIntervalRate);
            return retBuffer;
         }
      }
   }
   return "";
}

DefineEngineFunction(addMetric, void, (const char* metricsGroup, const char* metricsVariable, const char* metricDescription, bool graph, LinearColorF graphColor, S32 graphIntervalRate), ("", "", "", false, LinearColorF::WHITE, 0), "()"
   "@brief Adds a new metric to the tracker so it can be easily displayed via a UI or the console\n"
   "@param metricsGroup - The group to which the metric belongs. This is used to organize the metrics in the UI\n"
   "@param metricsVariable - The variable to be tracked as a metric\n"
   "@param graph - Is this variable meant to be visually graphed in the profiler\n"
   "@param graphColor - If graphed, what color is used\n"
   "@param graphIntervalRate - If graphed, what is the polling rate on the graph?\n"
   "@ingroup Utils")
{
   gMetricsTracker.addMetric(StringTable->insert(metricsGroup), metricsVariable, metricDescription, graph, graphColor, graphIntervalRate);
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

DefineEngineFunction(getMetricsGraphInfo, const char*, (const char* metricsGroup, S32 index), ("", -1), "()"
   "@brief Returns the graph info of the metric at the specified index\n"
   "@param metricsGroup - The group to which the metrics belong\n"
   "@param index - The index of the metric\n"
   "@ingroup Utils")
{
   return gMetricsTracker.getMetricsGraphInfo(StringTable->insert(metricsGroup), index);
}

