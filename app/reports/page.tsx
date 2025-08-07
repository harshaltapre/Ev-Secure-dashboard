"use client"

import { useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Badge } from "@/components/ui/badge"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs"
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select"
import { Calendar } from "@/components/ui/calendar"
import { Popover, PopoverContent, PopoverTrigger } from "@/components/ui/popover"
import { FileText, Download, CalendarIcon, Zap, Shield, DollarSign, Activity, FileDown, Printer } from "lucide-react"
import { format } from "date-fns"
import {
  Bar,
  BarChart,
  Line,
  ResponsiveContainer,
  XAxis,
  YAxis,
  CartesianGrid,
  Area,
  AreaChart,
  LineChart,
} from "recharts"
import { ChartContainer, ChartTooltip, ChartTooltipContent } from "@/components/ui/chart"
import { SharedNavigation } from "@/components/shared-navigation"

const monthlyReports = [
  { month: "Jan 2024", revenue: 45231, sessions: 1250, threats: 3, uptime: 99.2 },
  { month: "Feb 2024", revenue: 52341, sessions: 1456, threats: 1, uptime: 99.8 },
  { month: "Mar 2024", revenue: 48923, sessions: 1389, threats: 2, uptime: 99.5 },
  { month: "Apr 2024", revenue: 61234, sessions: 1678, threats: 0, uptime: 100 },
  { month: "May 2024", revenue: 58912, sessions: 1534, threats: 1, uptime: 99.7 },
  { month: "Jun 2024", revenue: 67845, sessions: 1823, threats: 2, uptime: 99.4 },
]

const stationPerformance = [
  { station: "ST001", revenue: 15420, sessions: 456, efficiency: 94.2 },
  { station: "ST002", revenue: 12890, sessions: 378, efficiency: 87.5 },
  { station: "ST003", revenue: 18750, sessions: 523, efficiency: 96.8 },
  { station: "ST004", revenue: 21785, sessions: 466, efficiency: 92.1 },
]

const threatAnalysis = [
  { type: "MITM Attack", count: 3, severity: "High", blocked: 3 },
  { type: "Voltage Spike", count: 5, severity: "Medium", blocked: 5 },
  { type: "Firmware Tampering", count: 2, severity: "Low", blocked: 1 },
  { type: "Unauthorized Access", count: 4, severity: "High", blocked: 4 },
]

const usagePatterns = [
  { hour: "00:00", usage: 12 },
  { hour: "06:00", usage: 45 },
  { hour: "12:00", usage: 89 },
  { hour: "18:00", usage: 156 },
  { hour: "24:00", usage: 78 },
]

export default function ReportsPage() {
  const [dateRange, setDateRange] = useState<Date | undefined>(new Date())
  const [reportType, setReportType] = useState("monthly")
  const [selectedStation, setSelectedStation] = useState("all")
  const [isGenerating, setIsGenerating] = useState(false)

  const generateReport = async (format: string) => {
    setIsGenerating(true)

    // Simulate report generation
    setTimeout(() => {
      const reportData = {
        type: reportType,
        station: selectedStation,
        date: dateRange,
        format: format,
        timestamp: new Date().toISOString(),
      }

      // Create and download file
      const dataStr =
        format === "json"
          ? JSON.stringify(reportData, null, 2)
          : `EV-Secure Report - ${reportType}\nGenerated: ${new Date().toLocaleString()}\nStation: ${selectedStation}\nFormat: ${format}`

      const dataBlob = new Blob([dataStr], { type: format === "json" ? "application/json" : "text/plain" })
      const url = URL.createObjectURL(dataBlob)
      const link = document.createElement("a")
      link.href = url
      link.download = `ev-secure-report-${reportType}-${Date.now()}.${format === "json" ? "json" : format}`
      document.body.appendChild(link)
      link.click()
      document.body.removeChild(link)
      URL.revokeObjectURL(url)

      setIsGenerating(false)
    }, 2000)
  }

  const printReport = () => {
    window.print()
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-purple-50 to-green-50">
      <SharedNavigation currentPage="reports" />

      <div className="lg:ml-64 p-4 sm:p-6 space-y-4 sm:space-y-6 max-w-full overflow-hidden">
        {/* Header */}
        <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
          <div className="min-w-0">
            <h1 className="text-2xl sm:text-3xl font-bold text-gray-800 flex items-center gap-2">
              <FileText className="w-6 h-6 sm:w-8 sm:h-8 text-blue-600 flex-shrink-0" />
              <span className="truncate">Analytics Reports</span>
            </h1>
            <p className="text-gray-600 mt-1 text-sm sm:text-base">Comprehensive insights and performance analytics</p>
          </div>
          <div className="flex items-center gap-2">
            <Popover>
              <PopoverTrigger asChild>
                <Button variant="outline" className="bg-white/80">
                  <CalendarIcon className="w-4 h-4 mr-2" />
                  {dateRange ? format(dateRange, "PPP") : "Pick a date"}
                </Button>
              </PopoverTrigger>
              <PopoverContent className="w-auto p-0">
                <Calendar mode="single" selected={dateRange} onSelect={setDateRange} initialFocus />
              </PopoverContent>
            </Popover>
            <Button onClick={printReport} variant="outline" className="bg-white/80">
              <Printer className="w-4 h-4 mr-2" />
              Print
            </Button>
          </div>
        </div>

        {/* Report Controls */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardContent className="p-4">
            <div className="flex flex-col sm:flex-row gap-4">
              <div className="flex-1">
                <label className="text-sm font-medium text-gray-700 mb-2 block">Report Type</label>
                <Select value={reportType} onValueChange={setReportType}>
                  <SelectTrigger className="bg-white/50">
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="monthly">Monthly Summary</SelectItem>
                    <SelectItem value="weekly">Weekly Analysis</SelectItem>
                    <SelectItem value="daily">Daily Reports</SelectItem>
                    <SelectItem value="custom">Custom Range</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div className="flex-1">
                <label className="text-sm font-medium text-gray-700 mb-2 block">Station Filter</label>
                <Select value={selectedStation} onValueChange={setSelectedStation}>
                  <SelectTrigger className="bg-white/50">
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="all">All Stations</SelectItem>
                    <SelectItem value="ST001">Downtown Plaza</SelectItem>
                    <SelectItem value="ST002">Mall Parking</SelectItem>
                    <SelectItem value="ST003">Airport Terminal</SelectItem>
                    <SelectItem value="ST004">University Campus</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div className="flex items-end gap-2">
                <Button
                  onClick={() => generateReport("pdf")}
                  disabled={isGenerating}
                  className="bg-gradient-to-r from-red-500 to-pink-500 text-white"
                >
                  {isGenerating ? (
                    <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin mr-2" />
                  ) : (
                    <FileDown className="w-4 h-4 mr-2" />
                  )}
                  PDF
                </Button>
                <Button
                  onClick={() => generateReport("csv")}
                  disabled={isGenerating}
                  className="bg-gradient-to-r from-green-500 to-blue-500 text-white"
                >
                  {isGenerating ? (
                    <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin mr-2" />
                  ) : (
                    <Download className="w-4 h-4 mr-2" />
                  )}
                  CSV
                </Button>
                <Button
                  onClick={() => generateReport("json")}
                  disabled={isGenerating}
                  className="bg-gradient-to-r from-purple-500 to-indigo-500 text-white"
                >
                  {isGenerating ? (
                    <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin mr-2" />
                  ) : (
                    <FileText className="w-4 h-4 mr-2" />
                  )}
                  JSON
                </Button>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Generate Report Section */}
        <Card className="bg-gradient-to-r from-blue-50 to-purple-50 border-blue-200 shadow-lg">
          <CardHeader>
            <CardTitle className="text-gray-800 flex items-center gap-2">
              <FileDown className="w-5 h-5 text-blue-600" />
              Generate Custom Report
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
              <div className="space-y-2">
                <h4 className="font-semibold text-gray-800">PDF Report</h4>
                <p className="text-sm text-gray-600">Comprehensive formatted report with charts and analysis</p>
                <Button
                  onClick={() => generateReport("pdf")}
                  disabled={isGenerating}
                  className="w-full bg-gradient-to-r from-red-500 to-pink-500 text-white"
                >
                  {isGenerating ? "Generating..." : "Download PDF"}
                </Button>
              </div>
              <div className="space-y-2">
                <h4 className="font-semibold text-gray-800">CSV Export</h4>
                <p className="text-sm text-gray-600">Raw data export for spreadsheet analysis</p>
                <Button
                  onClick={() => generateReport("csv")}
                  disabled={isGenerating}
                  className="w-full bg-gradient-to-r from-green-500 to-blue-500 text-white"
                >
                  {isGenerating ? "Generating..." : "Export CSV"}
                </Button>
              </div>
              <div className="space-y-2">
                <h4 className="font-semibold text-gray-800">JSON Data</h4>
                <p className="text-sm text-gray-600">Structured data for API integration</p>
                <Button
                  onClick={() => generateReport("json")}
                  disabled={isGenerating}
                  className="w-full bg-gradient-to-r from-purple-500 to-indigo-500 text-white"
                >
                  {isGenerating ? "Generating..." : "Export JSON"}
                </Button>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Report Tabs */}
        <Tabs defaultValue="overview" className="space-y-6">
          <TabsList className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg w-full sm:w-auto">
            <TabsTrigger value="overview" className="text-xs sm:text-sm">
              Overview
            </TabsTrigger>
            <TabsTrigger value="performance" className="text-xs sm:text-sm">
              Performance
            </TabsTrigger>
            <TabsTrigger value="security" className="text-xs sm:text-sm">
              Security
            </TabsTrigger>
            <TabsTrigger value="usage" className="text-xs sm:text-sm">
              Usage Patterns
            </TabsTrigger>
          </TabsList>

          <TabsContent value="overview" className="space-y-6">
            {/* Summary Cards */}
            <div className="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-6">
              <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
                <CardContent className="p-4">
                  <div className="flex items-center justify-between">
                    <div>
                      <p className="text-sm text-gray-600">Total Revenue</p>
                      <p className="text-2xl font-bold text-green-600">$354,385</p>
                    </div>
                    <DollarSign className="w-8 h-8 text-green-500" />
                  </div>
                  <p className="text-xs text-green-600 mt-2">+12.5% from last period</p>
                </CardContent>
              </Card>

              <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
                <CardContent className="p-4">
                  <div className="flex items-center justify-between">
                    <div>
                      <p className="text-sm text-gray-600">Total Sessions</p>
                      <p className="text-2xl font-bold text-blue-600">9,130</p>
                    </div>
                    <Activity className="w-8 h-8 text-blue-500" />
                  </div>
                  <p className="text-xs text-blue-600 mt-2">+8.3% from last period</p>
                </CardContent>
              </Card>

              <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
                <CardContent className="p-4">
                  <div className="flex items-center justify-between">
                    <div>
                      <p className="text-sm text-gray-600">Avg Uptime</p>
                      <p className="text-2xl font-bold text-purple-600">99.6%</p>
                    </div>
                    <Zap className="w-8 h-8 text-purple-500" />
                  </div>
                  <p className="text-xs text-purple-600 mt-2">+0.4% from last period</p>
                </CardContent>
              </Card>

              <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
                <CardContent className="p-4">
                  <div className="flex items-center justify-between">
                    <div>
                      <p className="text-sm text-gray-600">Threats Blocked</p>
                      <p className="text-2xl font-bold text-red-600">9</p>
                    </div>
                    <Shield className="w-8 h-8 text-red-500" />
                  </div>
                  <p className="text-xs text-red-600 mt-2">-25% from last period</p>
                </CardContent>
              </Card>
            </div>

            {/* Monthly Trends */}
            <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
              <CardHeader>
                <CardTitle className="text-gray-800">Monthly Performance Trends</CardTitle>
              </CardHeader>
              <CardContent>
                <ChartContainer
                  config={{
                    revenue: { label: "Revenue ($)", color: "#10b981" },
                    sessions: { label: "Sessions", color: "#3b82f6" },
                  }}
                  className="h-[300px] sm:h-[400px] w-full"
                >
                  <ResponsiveContainer width="100%" height="100%">
                    <AreaChart data={monthlyReports}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="month" />
                      <YAxis />
                      <ChartTooltip content={<ChartTooltipContent />} />
                      <Area type="monotone" dataKey="revenue" stroke="#10b981" fill="#10b981" fillOpacity={0.3} />
                      <Area type="monotone" dataKey="sessions" stroke="#3b82f6" fill="#3b82f6" fillOpacity={0.3} />
                    </AreaChart>
                  </ResponsiveContainer>
                </ChartContainer>
              </CardContent>
            </Card>
          </TabsContent>

          <TabsContent value="performance" className="space-y-6">
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
              <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
                <CardHeader>
                  <CardTitle className="text-gray-800">Station Performance</CardTitle>
                </CardHeader>
                <CardContent>
                  <ChartContainer
                    config={{
                      revenue: { label: "Revenue", color: "#8b5cf6" },
                      sessions: { label: "Sessions", color: "#10b981" },
                    }}
                    className="h-[300px] w-full"
                  >
                    <ResponsiveContainer width="100%" height="100%">
                      <BarChart data={stationPerformance}>
                        <CartesianGrid strokeDasharray="3 3" />
                        <XAxis dataKey="station" />
                        <YAxis />
                        <ChartTooltip content={<ChartTooltipContent />} />
                        <Bar dataKey="revenue" fill="#8b5cf6" />
                        <Bar dataKey="sessions" fill="#10b981" />
                      </BarChart>
                    </ResponsiveContainer>
                  </ChartContainer>
                </CardContent>
              </Card>

              <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
                <CardHeader>
                  <CardTitle className="text-gray-800">Efficiency Metrics</CardTitle>
                </CardHeader>
                <CardContent className="space-y-4">
                  {stationPerformance.map((station) => (
                    <div key={station.station} className="space-y-2">
                      <div className="flex justify-between text-sm">
                        <span className="font-medium">{station.station}</span>
                        <span>{station.efficiency}%</span>
                      </div>
                      <div className="w-full bg-gray-200 rounded-full h-2">
                        <div
                          className="bg-gradient-to-r from-green-400 to-blue-500 h-2 rounded-full"
                          style={{ width: `${station.efficiency}%` }}
                        ></div>
                      </div>
                    </div>
                  ))}
                </CardContent>
              </Card>
            </div>
          </TabsContent>

          <TabsContent value="security" className="space-y-6">
            <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
              <CardHeader>
                <CardTitle className="text-gray-800">Threat Analysis</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  {threatAnalysis.map((threat, index) => (
                    <div
                      key={index}
                      className="flex items-center justify-between p-4 bg-gradient-to-r from-red-50 to-orange-50 rounded-lg border border-red-100"
                    >
                      <div>
                        <h4 className="font-semibold text-gray-800">{threat.type}</h4>
                        <p className="text-sm text-gray-600">
                          {threat.count} incidents • {threat.blocked} blocked
                        </p>
                      </div>
                      <Badge
                        variant={
                          threat.severity === "High"
                            ? "destructive"
                            : threat.severity === "Medium"
                              ? "default"
                              : "secondary"
                        }
                      >
                        {threat.severity}
                      </Badge>
                    </div>
                  ))}
                </div>
              </CardContent>
            </Card>
          </TabsContent>

          <TabsContent value="usage" className="space-y-6">
            <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
              <CardHeader>
                <CardTitle className="text-gray-800">Daily Usage Patterns</CardTitle>
              </CardHeader>
              <CardContent>
                <ChartContainer
                  config={{
                    usage: { label: "Usage (%)", color: "#f59e0b" },
                  }}
                  className="h-[300px] sm:h-[400px] w-full"
                >
                  <ResponsiveContainer width="100%" height="100%">
                    <LineChart data={usagePatterns}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="hour" />
                      <YAxis />
                      <ChartTooltip content={<ChartTooltipContent />} />
                      <Line
                        type="monotone"
                        dataKey="usage"
                        stroke="#f59e0b"
                        strokeWidth={3}
                        dot={{ fill: "#f59e0b", strokeWidth: 2, r: 6 }}
                      />
                    </LineChart>
                  </ResponsiveContainer>
                </ChartContainer>
              </CardContent>
            </Card>
          </TabsContent>
        </Tabs>
      </div>
    </div>
  )
}
