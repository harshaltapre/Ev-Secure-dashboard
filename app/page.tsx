"use client"

import { useState, useEffect } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Progress } from "@/components/ui/progress"
import {
  Zap,
  Shield,
  AlertTriangle,
  CheckCircle,
  Car,
  MapPin,
  FileText,
  Users,
  RefreshCw,
  Settings,
  TrendingUp,
  Activity,
  Battery,
  Wifi,
} from "lucide-react"
import { SharedNavigation } from "@/components/shared-navigation"
import { useRouter } from "next/navigation"

const dashboardStats = {
  totalStations: 6,
  activeStations: 0, // Set to 0 when no ESP32 connected
  connectedCars: 0,
  mlThreats: 0,
  totalRevenue: 0,
  totalSessions: 0,
  avgUptime: 0,
  systemHealth: 0,
}

const recentActivity = [
  {
    id: 1,
    type: "system_status",
    message: "System waiting for ESP32-S3 connection",
    timestamp: "Now",
    icon: AlertTriangle,
    color: "text-orange-600",
  },
  {
    id: 2,
    type: "api_status",
    message: "API keys ready for configuration",
    timestamp: "System startup",
    icon: Shield,
    color: "text-blue-600",
  },
]

const quickActions = [
  {
    title: "View Station Map",
    description: "See all stations on interactive map",
    icon: MapPin,
    href: "/map",
    gradient: "from-blue-500 to-purple-500",
  },
  {
    title: "Generate Report",
    description: "Download PDF/CSV reports",
    icon: FileText,
    href: "/reports",
    gradient: "from-green-500 to-blue-500",
  },
  {
    title: "Manage API Keys",
    description: "Configure ESP32-S3 authentication",
    icon: Users,
    href: "/settings",
    gradient: "from-purple-500 to-pink-500",
  },
]

export default function DashboardPage() {
  const [liveUpdates, setLiveUpdates] = useState(true)
  const [lastRefresh, setLastRefresh] = useState(new Date())
  const [isConnected, setIsConnected] = useState(false) // ESP32 connection status
  const router = useRouter()

  useEffect(() => {
    // Check ESP32 connection status
    const checkConnection = () => {
      // In real app, this would check actual ESP32 connection
      setIsConnected(false) // Set to false to show "No Data Available" state
    }

    checkConnection()
    const interval = setInterval(() => {
      if (liveUpdates && isConnected) {
        setLastRefresh(new Date())
      }
    }, 3000)

    return () => clearInterval(interval)
  }, [liveUpdates, isConnected])

  const handleRefresh = () => {
    setLastRefresh(new Date())
    // Force page reload to get fresh data
    window.location.reload()
  }

  const handleSettings = () => {
    router.push("/settings")
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-purple-50 to-green-50">
      <SharedNavigation currentPage="dashboard" />

      <div className="lg:ml-64 p-4 sm:p-6 space-y-4 sm:space-y-6 max-w-full overflow-hidden">
        {/* Header with Live Updates */}
        <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
          <div className="min-w-0">
            <h1 className="text-2xl sm:text-3xl font-bold text-gray-800 flex items-center gap-2">
              <Zap className="w-6 h-6 sm:w-8 sm:h-8 text-purple-600 flex-shrink-0" />
              <span className="truncate">EV-Secure Dashboard</span>
            </h1>
            <p className="text-gray-600 mt-1 text-sm sm:text-base">Real-time monitoring and control center</p>
          </div>
          <div className="flex items-center gap-2 sm:gap-3">
            <div className="flex items-center gap-2 px-3 py-2 bg-orange-100 rounded-full">
              <div className="w-2 h-2 bg-orange-500 rounded-full"></div>
              <span className="text-sm font-medium text-orange-700">
                {isConnected ? "Live Updates" : "Waiting for ESP32"}
              </span>
            </div>
            <Button variant="outline" onClick={handleRefresh} className="bg-white/80">
              <RefreshCw className="w-4 h-4 mr-2" />
              Refresh
            </Button>
            <Button onClick={handleSettings} className="bg-gradient-to-r from-green-500 to-purple-500 text-white">
              <Settings className="w-4 h-4 mr-2" />
              Settings
            </Button>
          </div>
        </div>

        {/* ESP32 Connection Alert */}
        {!isConnected && (
          <div className="bg-orange-50 border border-orange-200 rounded-lg p-4">
            <div className="flex items-start space-x-3">
              <AlertTriangle className="w-5 h-5 text-orange-600 flex-shrink-0 mt-0.5" />
              <div>
                <h4 className="font-semibold text-orange-800 mb-1">ESP32-S3 Not Connected</h4>
                <p className="text-sm text-orange-700">
                  No charging stations are currently connected. Configure your ESP32-S3 devices with valid API keys to
                  see real-time data.
                </p>
              </div>
            </div>
          </div>
        )}

        {/* Stats Cards */}
        <div className="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-6">
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg hover:shadow-xl transition-all duration-300">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">Total Stations</p>
                  <p className="text-2xl font-bold text-gray-800">
                    {isConnected ? dashboardStats.totalStations : "No Data"}
                  </p>
                </div>
                <div className="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center">
                  <Zap className="w-6 h-6 text-blue-600" />
                </div>
              </div>
              <div className="mt-2">
                <Progress value={isConnected ? 85 : 0} className="h-2" />
                <p className="text-xs text-gray-500 mt-1">{isConnected ? "85% operational" : "ESP32 required"}</p>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg hover:shadow-xl transition-all duration-300">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">Active Stations</p>
                  <p className="text-2xl font-bold text-green-600">
                    {isConnected ? dashboardStats.activeStations : "No Data"}
                  </p>
                </div>
                <div className="w-12 h-12 bg-green-100 rounded-lg flex items-center justify-center">
                  <CheckCircle className="w-6 h-6 text-green-600" />
                </div>
              </div>
              <div className="mt-2">
                <Progress value={isConnected ? 67 : 0} className="h-2" />
                <p className="text-xs text-gray-500 mt-1">{isConnected ? "+12% from yesterday" : "ESP32 required"}</p>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg hover:shadow-xl transition-all duration-300">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">Connected Cars</p>
                  <p className="text-2xl font-bold text-blue-600">
                    {isConnected ? dashboardStats.connectedCars : "No Data"}
                  </p>
                </div>
                <div className="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center">
                  <Car className="w-6 h-6 text-blue-600" />
                </div>
              </div>
              <div className="mt-2">
                <Progress value={isConnected ? 75 : 0} className="h-2" />
                <p className="text-xs text-gray-500 mt-1">{isConnected ? "Peak usage: 6 cars" : "ESP32 required"}</p>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg hover:shadow-xl transition-all duration-300">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">ML Threats</p>
                  <p className="text-2xl font-bold text-red-600">
                    {isConnected ? dashboardStats.mlThreats : "No Data"}
                  </p>
                </div>
                <div className="w-12 h-12 bg-red-100 rounded-lg flex items-center justify-center">
                  <Shield className="w-6 h-6 text-red-600" />
                </div>
              </div>
              <div className="mt-2">
                <Progress value={isConnected ? 20 : 0} className="h-2" />
                <p className="text-xs text-gray-500 mt-1">{isConnected ? "-50% from last week" : "ESP32 required"}</p>
              </div>
            </CardContent>
          </Card>
        </div>

        {/* Quick Actions */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardHeader>
            <CardTitle className="text-gray-800">Quick Actions</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
              {quickActions.map((action, index) => (
                <a
                  key={index}
                  href={action.href}
                  className="group block p-4 rounded-xl border border-gray-200 hover:border-purple-300 transition-all duration-200 hover:shadow-lg hover:scale-105"
                >
                  <div className="flex items-center space-x-3">
                    <div
                      className={`w-12 h-12 rounded-lg bg-gradient-to-r ${action.gradient} flex items-center justify-center`}
                    >
                      <action.icon className="w-6 h-6 text-white" />
                    </div>
                    <div>
                      <h3 className="font-semibold text-gray-800 group-hover:text-purple-600 transition-colors">
                        {action.title}
                      </h3>
                      <p className="text-sm text-gray-600">{action.description}</p>
                    </div>
                  </div>
                </a>
              ))}
            </div>
          </CardContent>
        </Card>

        {/* Performance Overview & Recent Activity */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardHeader>
              <CardTitle className="text-gray-800 flex items-center gap-2">
                <TrendingUp className="w-5 h-5 text-green-600" />
                Performance Overview
              </CardTitle>
            </CardHeader>
            <CardContent className="space-y-4">
              {isConnected ? (
                <div className="space-y-3">
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span className="text-gray-600">System Health</span>
                      <span className="font-medium">{dashboardStats.systemHealth}%</span>
                    </div>
                    <Progress value={dashboardStats.systemHealth} className="h-2" />
                  </div>
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span className="text-gray-600">Network Uptime</span>
                      <span className="font-medium">{dashboardStats.avgUptime}%</span>
                    </div>
                    <Progress value={dashboardStats.avgUptime} className="h-2" />
                  </div>
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span className="text-gray-600">API Response</span>
                      <span className="font-medium">98.5%</span>
                    </div>
                    <Progress value={98.5} className="h-2" />
                  </div>
                </div>
              ) : (
                <div className="text-center py-8">
                  <AlertTriangle className="w-12 h-12 text-gray-400 mx-auto mb-4" />
                  <h3 className="text-lg font-semibold text-gray-800 mb-2">No Performance Data</h3>
                  <p className="text-gray-600 mb-4">Connect ESP32-S3 stations to view performance metrics.</p>
                </div>
              )}
              <div className="pt-4 border-t border-gray-100">
                <div className="grid grid-cols-2 gap-4 text-center">
                  <div>
                    <p className="text-2xl font-bold text-green-600">
                      {isConnected ? `$${dashboardStats.totalRevenue.toLocaleString()}` : "No Data"}
                    </p>
                    <p className="text-sm text-gray-600">Revenue</p>
                  </div>
                  <div>
                    <p className="text-2xl font-bold text-blue-600">
                      {isConnected ? dashboardStats.totalSessions : "No Data"}
                    </p>
                    <p className="text-sm text-gray-600">Sessions</p>
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardHeader>
              <CardTitle className="text-gray-800 flex items-center gap-2">
                <Activity className="w-5 h-5 text-purple-600" />
                Recent Activity
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {recentActivity.map((activity) => (
                  <div key={activity.id} className="flex items-start space-x-3">
                    <div className="flex-shrink-0">
                      <activity.icon className={`w-5 h-5 ${activity.color}`} />
                    </div>
                    <div className="flex-1 min-w-0">
                      <p className="text-sm font-medium text-gray-800">{activity.message}</p>
                      <p className="text-xs text-gray-500">{activity.timestamp}</p>
                    </div>
                  </div>
                ))}
              </div>
              <div className="mt-4 pt-4 border-t border-gray-100">
                <Button variant="outline" className="w-full bg-white/50">
                  Configure ESP32-S3 Connection
                </Button>
              </div>
            </CardContent>
          </Card>
        </div>

        {/* System Status */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardHeader>
            <CardTitle className="text-gray-800">System Status</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
              <div className="flex items-center space-x-3 p-3 bg-green-50 rounded-lg">
                <Wifi className="w-5 h-5 text-green-600" />
                <div>
                  <p className="text-sm font-medium text-green-800">Network</p>
                  <p className="text-xs text-green-600">Online</p>
                </div>
              </div>
              <div className="flex items-center space-x-3 p-3 bg-blue-50 rounded-lg">
                <Shield className="w-5 h-5 text-blue-600" />
                <div>
                  <p className="text-sm font-medium text-blue-800">Security</p>
                  <p className="text-xs text-blue-600">Protected</p>
                </div>
              </div>
              <div className="flex items-center space-x-3 p-3 bg-orange-50 rounded-lg">
                <Activity className="w-5 h-5 text-orange-600" />
                <div>
                  <p className="text-sm font-medium text-orange-800">ESP32-S3</p>
                  <p className="text-xs text-orange-600">{isConnected ? "Connected" : "Disconnected"}</p>
                </div>
              </div>
              <div className="flex items-center space-x-3 p-3 bg-purple-50 rounded-lg">
                <Battery className="w-5 h-5 text-purple-600" />
                <div>
                  <p className="text-sm font-medium text-purple-800">API Status</p>
                  <p className="text-xs text-purple-600">Ready</p>
                </div>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Last Updated */}
        <div className="text-center text-sm text-gray-500">
          Last updated: {lastRefresh.toLocaleTimeString()}
          {!isConnected && " • Waiting for ESP32-S3 connection"}
        </div>
      </div>
    </div>
  )
}
