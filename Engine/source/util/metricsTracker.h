#ifndef _UTIL_METRICSTRACKER_H_
#define _UTIL_METRICSTRACKER_H_

#include "core/util/tDictionary.h"
#include "core/util/tVector.h"
#include "platform/platform.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "core/color.h"

struct MetricsTracker
{
private:
   struct MetricsVariable
   {
      String metricsVariable;
      String metricsDescription;
      bool graph;
      LinearColorF graphColor;
      S32 graphIntervalRate;
   };

   struct MetricsEntry
   {
      StringTableEntry metricsGroup{};

      //We use this instead of a Map to ensure that the order of registration is preserved
      Vector<MetricsVariable> metricsVarEntry;
   };

   Vector<MetricsEntry> mMetricsList;

public:
   MetricsTracker();

   /// <summary>
   /// Adds a new metric to the tracker so it can be easily displayed via a UI or the console
   /// <param> metricsGroup - The group to which the metric belongs. This is used to organize the metrics in the UI
   /// <param> metricsVariable - The variable to be tracked as a metric
   /// <param> metricsDescription - A description of the metric
   /// <param> graph - Is this variable meant to be visually graphed in the profiler
   /// <param> graphColor - If graphed, what color is used
   /// <param> graphIntervalRate - If graphed, what is the polling rate on the graph?
   /// </summary>
   void addMetric(StringTableEntry metricsGroup, const String& metricsVariable, const String& metricsDescription, const bool& graph = false,
      const LinearColorF& graphColor = LinearColorF::WHITE, const S32& graphIntervalRate = 0);

   /// <summary>
   /// Removes a metric from the tracker
   /// <param> metricsGroup - The group to which the metric belongs
   /// <param> metricsVariable - The variable to be removed from the tracker
   /// </summary>
   void removeMetric(StringTableEntry metricsGroup, const String& metricsVariable);

   /// <summary>
   /// Returns the number of metrics in the specified group
   /// <param> metricsGroup - The group to which the metrics belong
   /// </summary>
   S32 getMetricsCount(StringTableEntry metricsGroup);

   /// <summary>
   /// Returns the variable name of the metric at the specified index
   /// <param> metricsGroup - The group to which the metrics belong
   /// <param> index - The index of the metric
   /// </summary>
   const char* getMetricsVariable(StringTableEntry metricsGroup, const S32& index);

   /// <summary>
   /// Returns the description of the metric at the specified index
   /// <param> metricsGroup - The group to which the metrics belong
   /// <param> index - The index of the metric
   /// </summary>
   const char* getMetricsDescription(StringTableEntry metricsGroup, const S32& index);

   /// <summary>
   /// Returns the graph info of the metric at the specified index
   /// <param> metricsGroup - The group to which the metrics belong
   /// <param> index - The index of the metric
   /// </summary>
   const char* getMetricsGraphInfo(StringTableEntry metricsGroup, const S32& index);
};

extern MetricsTracker gMetricsTracker;

#endif
