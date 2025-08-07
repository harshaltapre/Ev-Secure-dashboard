"use client"

import { useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Switch } from "@/components/ui/switch"
import {
  Zap,
  Power,
  Wifi,
  WifiOff,
  Search,
  Filter,
  RefreshCw,
  AlertTriangle,
  CheckCircle,
  XCircle,
  Settings,
  MoreVertical,
  Car,
  Cpu,
  Battery,
  Thermometer,
} from "lucide-react"
import { DropdownMenu, DropdownMenuContent, DropdownMenuItem, DropdownMenuTrigger } from "@/components/ui/dropdown-menu"
import { Progress } from "@/components/ui/progress"
import { SharedNavigation } from "@/components/shared-navigation"

const stationsData = [
  {
    id: "ST001",
    name: "Downtown Plaza",
    status: "active",
    voltage: 240.15,
    current: 32.45,
    temp: 28.5,
    power: 7.8,
    location: "Downtown District",
    address: "123 Main St, City Center",
    apiKey: "active",
    lastSync: "2 min ago",
    relayStatus: true,
    firmwareVersion: "v2.1.3",
    connectedUser: "user@email.com",
    carConnected: true,
    carModel: "Tesla Model 3",
    chargingProgress: 65,
    mlThreatLevel: "safe",
    esp32Status: "online",
    batteryLevel: 85,
  },
  {
    id: "ST002",
    name: "Mall Parking",
    status: "threat",
    voltage: 238.92,
    current: 31.78,
    temp: 35.2,
    power: 7.6,
    location: "Shopping District",
    address: "456 Commerce Ave, Mall Plaza",
    apiKey: "active",
    lastSync: "1 min ago",
    relayStatus: false,
    firmwareVersion: "v2.1.2",
    connectedUser: null,
    carConnected: true,
    carModel: "BMW i4",
    chargingProgress: 0,
    mlThreatLevel: "high",
    esp32Status: "alert",
    batteryLevel: 92,
  },
  {
    id: "ST003",
    name: "Airport Terminal",
    status: "offline",
    voltage: 0,
    current: 0,
    temp: 22.1,
    power: 0,
    location: "Airport",
    address: "789 Airport Rd, Terminal B",
    apiKey: "inactive",
    lastSync: "15 min ago",
    relayStatus: false,
    firmwareVersion: "v2.0.8",
    connectedUser: null,
    carConnected: false,
    carModel: null,
    chargingProgress: 0,
    mlThreatLevel: "unknown",
    esp32Status: "offline",
    batteryLevel: 0,
  },
  {
    id: "ST004",
    name: "University Campus",
    status: "active",
    voltage: 241.33,
    current: 28.91,
    temp: 26.8,
    power: 6.9,
    location: "Education District",
    address: "321 University Blvd, Campus North",
    apiKey: "active",
    lastSync: "30 sec ago",
    relayStatus: true,
    firmwareVersion: "v2.1.3",
    connectedUser: "student@university.edu",
    carConnected: true,
    carModel: "Nissan Leaf",
    chargingProgress: 45,
    mlThreatLevel: "safe",
    esp32Status: "online",
    batteryLevel: 78,
  },
  {
    id: "ST005",
    name: "Business Park",
    status: "active",
    voltage: 239.87,
    current: 30.12,
    temp: 29.4,
    power: 7.2,
    location: "Business District",
    address: "555 Corporate Dr, Business Park",
    apiKey: "active",
    lastSync: "1 min ago",
    relayStatus: true,
    firmwareVersion: "v2.1.3",
    connectedUser: "employee@company.com",
    carConnected: false,
    carModel: null,
    chargingProgress: 0,
    mlThreatLevel: "safe",
    esp32Status: "online",
    batteryLevel: 88,
  },
  {
    id: "ST006",
    name: "Residential Complex",
    status: "active",
    voltage: 240.55,
    current: 25.67,
    temp: 27.2,
    power: 6.2,
    location: "Residential Area",
    address: "888 Home Ave, Residential Complex",
    apiKey: "active",
    lastSync: "45 sec ago",
    relayStatus: true,
    firmwareVersion: "v2.1.3",
    connectedUser: "resident@home.com",
    carConnected: true,
    carModel: "Audi e-tron",
    chargingProgress: 80,
    mlThreatLevel: "safe",
    esp32Status: "online",
    batteryLevel: 91,
  },
]

export default function StationsPage() {
  const [stations, setStations] = useState(stationsData)
  const [searchTerm, setSearchTerm] = useState("")
  const [filterStatus, setFilterStatus] = useState("all")

  const filteredStations = stations.filter((station) => {
    const matchesSearch =
      station.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      station.id.toLowerCase().includes(searchTerm.toLowerCase()) ||
      station.location.toLowerCase().includes(searchTerm.toLowerCase())
    const matchesFilter = filterStatus === "all" || station.status === filterStatus
    return matchesSearch && matchesFilter
  })

  const toggleRelay = (stationId: string) => {
    setStations((prev) =>
      prev.map((station) => (station.id === stationId ? { ...station, relayStatus: !station.relayStatus } : station)),
    )
  }

  const getStatusIcon = (status: string) => {
    switch (status) {
      case "active":
        return <CheckCircle className="w-4 h-4 sm:w-5 sm:h-5 text-green-500 flex-shrink-0" />
      case "threat":
        return <AlertTriangle className="w-4 h-4 sm:w-5 sm:h-5 text-red-500 flex-shrink-0" />
      case "offline":
        return <XCircle className="w-4 h-4 sm:w-5 sm:h-5 text-gray-500 flex-shrink-0" />
      default:
        return <XCircle className="w-4 h-4 sm:w-5 sm:h-5 text-gray-500 flex-shrink-0" />
    }
  }

  const getStatusColor = (status: string) => {
    switch (status) {
      case "active":
        return "bg-green-100 text-green-800 border-green-200"
      case "threat":
        return "bg-red-100 text-red-800 border-red-200"
      case "offline":
        return "bg-gray-100 text-gray-800 border-gray-200"
      default:
        return "bg-gray-100 text-gray-800 border-gray-200"
    }
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-purple-50 to-green-50">
      <SharedNavigation currentPage="stations" />

      <div className="lg:ml-64 p-4 sm:p-6 space-y-4 sm:space-y-6 max-w-full overflow-hidden">
        {/* Header */}
        <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
          <div className="min-w-0">
            <h1 className="text-2xl sm:text-3xl font-bold text-gray-800 flex items-center gap-2">
              <Zap className="w-6 h-6 sm:w-8 sm:h-8 text-purple-600 flex-shrink-0" />
              <span className="truncate">Charging Stations</span>
            </h1>
            <p className="text-gray-600 mt-1 text-sm sm:text-base">
              Monitor and control your EV charging infrastructure
            </p>
          </div>
          <div className="flex items-center gap-2">
            <Button variant="outline" size="sm" className="bg-white/80 text-xs sm:text-sm">
              <RefreshCw className="w-3 h-3 sm:w-4 sm:h-4 mr-1 sm:mr-2" />
              <span className="hidden sm:inline">Refresh</span>
            </Button>
            <Button size="sm" className="bg-gradient-to-r from-green-500 to-purple-500 text-white text-xs sm:text-sm">
              <Settings className="w-3 h-3 sm:w-4 sm:h-4 mr-1 sm:mr-2" />
              <span className="hidden sm:inline">Configure</span>
            </Button>
          </div>
        </div>

        {/* Stats Cards */}
        <div className="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-4">
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-3 sm:p-4">
              <div className="flex items-center justify-between">
                <div className="min-w-0">
                  <p className="text-xs sm:text-sm text-gray-600 truncate">Total Stations</p>
                  <p className="text-lg sm:text-2xl font-bold text-gray-800">{stations.length}</p>
                </div>
                <div className="w-8 h-8 sm:w-12 sm:h-12 bg-blue-100 rounded-lg flex items-center justify-center flex-shrink-0">
                  <Zap className="w-4 h-4 sm:w-6 sm:h-6 text-blue-600" />
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-3 sm:p-4">
              <div className="flex items-center justify-between">
                <div className="min-w-0">
                  <p className="text-xs sm:text-sm text-gray-600 truncate">Active</p>
                  <p className="text-lg sm:text-2xl font-bold text-green-600">
                    {stations.filter((s) => s.status === "active").length}
                  </p>
                </div>
                <div className="w-8 h-8 sm:w-12 sm:h-12 bg-green-100 rounded-lg flex items-center justify-center flex-shrink-0">
                  <CheckCircle className="w-4 h-4 sm:w-6 sm:h-6 text-green-600" />
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-3 sm:p-4">
              <div className="flex items-center justify-between">
                <div className="min-w-0">
                  <p className="text-xs sm:text-sm text-gray-600 truncate">Connected Cars</p>
                  <p className="text-lg sm:text-2xl font-bold text-blue-600">
                    {stations.filter((s) => s.carConnected).length}
                  </p>
                </div>
                <div className="w-8 h-8 sm:w-12 sm:h-12 bg-blue-100 rounded-lg flex items-center justify-center flex-shrink-0">
                  <Car className="w-4 h-4 sm:w-6 sm:h-6 text-blue-600" />
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-3 sm:p-4">
              <div className="flex items-center justify-between">
                <div className="min-w-0">
                  <p className="text-xs sm:text-sm text-gray-600 truncate">ML Threats</p>
                  <p className="text-lg sm:text-2xl font-bold text-red-600">
                    {stations.filter((s) => s.mlThreatLevel === "high").length}
                  </p>
                </div>
                <div className="w-8 h-8 sm:w-12 sm:h-12 bg-red-100 rounded-lg flex items-center justify-center flex-shrink-0">
                  <AlertTriangle className="w-4 h-4 sm:w-6 sm:h-6 text-red-600" />
                </div>
              </div>
            </CardContent>
          </Card>
        </div>

        {/* Search and Filter */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardContent className="p-3 sm:p-4">
            <div className="flex flex-col sm:flex-row gap-3 sm:gap-4">
              <div className="relative flex-1">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 w-4 h-4 text-gray-400" />
                <Input
                  placeholder="Search stations by name, ID, or location..."
                  value={searchTerm}
                  onChange={(e) => setSearchTerm(e.target.value)}
                  className="pl-10 bg-white/50 text-sm"
                />
              </div>
              <div className="flex items-center gap-2">
                <Filter className="w-4 h-4 text-gray-500 flex-shrink-0" />
                <select
                  value={filterStatus}
                  onChange={(e) => setFilterStatus(e.target.value)}
                  className="px-3 py-2 bg-white/50 border border-gray-200 rounded-lg text-sm min-w-0"
                >
                  <option value="all">All Status</option>
                  <option value="active">Active</option>
                  <option value="threat">Threats</option>
                  <option value="offline">Offline</option>
                </select>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Stations Grid */}
        <div className="grid grid-cols-1 lg:grid-cols-2 xl:grid-cols-3 gap-4 sm:gap-6">
          {filteredStations.map((station) => (
            <Card
              key={station.id}
              className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg hover:shadow-xl transition-all duration-300"
            >
              <CardHeader className="pb-3">
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2 sm:gap-3 min-w-0">
                    {getStatusIcon(station.status)}
                    <div className="min-w-0">
                      <CardTitle className="text-sm sm:text-lg text-gray-800 truncate">{station.name}</CardTitle>
                      <p className="text-xs sm:text-sm text-gray-600 truncate">
                        {station.id} • {station.location}
                      </p>
                    </div>
                  </div>
                  <div className="flex items-center gap-1 sm:gap-2 flex-shrink-0">
                    <Badge className={`${getStatusColor(station.status)} border text-xs`}>
                      {station.status.toUpperCase()}
                    </Badge>
                    <DropdownMenu>
                      <DropdownMenuTrigger asChild>
                        <Button variant="ghost" size="icon" className="w-6 h-6 sm:w-8 sm:h-8">
                          <MoreVertical className="w-3 h-3 sm:w-4 sm:h-4" />
                        </Button>
                      </DropdownMenuTrigger>
                      <DropdownMenuContent align="end">
                        <DropdownMenuItem>View Details</DropdownMenuItem>
                        <DropdownMenuItem>Update Firmware</DropdownMenuItem>
                        <DropdownMenuItem>Reset Station</DropdownMenuItem>
                      </DropdownMenuContent>
                    </DropdownMenu>
                  </div>
                </div>
              </CardHeader>
              <CardContent className="space-y-4">
                <div className="text-xs text-gray-600 bg-gray-50 p-2 rounded truncate">📍 {station.address}</div>

                {/* Car Connection Status */}
                {station.carConnected ? (
                  <div className="p-3 bg-gradient-to-r from-blue-50 to-green-50 rounded-lg border border-blue-100">
                    <div className="flex items-center justify-between mb-2">
                      <div className="flex items-center space-x-2 min-w-0">
                        <Car className="w-4 h-4 text-blue-600 flex-shrink-0" />
                        <span className="text-sm font-medium text-blue-800 truncate">{station.carModel}</span>
                      </div>
                      <span className="text-xs text-blue-600 flex-shrink-0">{station.chargingProgress}%</span>
                    </div>
                    <Progress value={station.chargingProgress} className="h-2" />
                  </div>
                ) : (
                  <div className="p-3 bg-gray-50 rounded-lg border border-gray-200">
                    <div className="flex items-center space-x-2">
                      <Car className="w-4 h-4 text-gray-400 flex-shrink-0" />
                      <span className="text-sm text-gray-600">No car connected</span>
                    </div>
                  </div>
                )}

                {/* ESP32 ML Status */}
                <div className="p-3 bg-gradient-to-r from-purple-50 to-pink-50 rounded-lg border border-purple-100">
                  <div className="flex items-center justify-between">
                    <div className="flex items-center space-x-2 min-w-0">
                      <Cpu className="w-4 h-4 text-purple-600 flex-shrink-0" />
                      <span className="text-sm font-medium text-purple-800 truncate">ESP32 ML</span>
                    </div>
                    <Badge
                      variant={
                        station.mlThreatLevel === "safe"
                          ? "default"
                          : station.mlThreatLevel === "high"
                            ? "destructive"
                            : "secondary"
                      }
                      className="text-xs flex-shrink-0"
                    >
                      {station.mlThreatLevel.toUpperCase()}
                    </Badge>
                  </div>
                </div>

                {/* Metrics Grid */}
                <div className="grid grid-cols-2 gap-2 sm:gap-3">
                  <div className="text-center p-2 sm:p-3 bg-gradient-to-br from-blue-50 to-blue-100 rounded-lg">
                    <p className="text-xs text-blue-600 font-medium">Voltage</p>
                    <p className="text-sm sm:text-lg font-bold text-blue-800">{station.voltage}V</p>
                  </div>
                  <div className="text-center p-2 sm:p-3 bg-gradient-to-br from-green-50 to-green-100 rounded-lg">
                    <p className="text-xs text-green-600 font-medium">Current</p>
                    <p className="text-sm sm:text-lg font-bold text-green-800">{station.current}A</p>
                  </div>
                  <div className="text-center p-2 sm:p-3 bg-gradient-to-br from-orange-50 to-orange-100 rounded-lg">
                    <div className="flex items-center justify-center space-x-1">
                      <Thermometer className="w-3 h-3 text-orange-600" />
                      <p className="text-xs text-orange-600 font-medium">Temp</p>
                    </div>
                    <p className="text-sm sm:text-lg font-bold text-orange-800">{station.temp}°C</p>
                  </div>
                  <div className="text-center p-2 sm:p-3 bg-gradient-to-br from-purple-50 to-purple-100 rounded-lg">
                    <div className="flex items-center justify-center space-x-1">
                      <Battery className="w-3 h-3 text-purple-600" />
                      <p className="text-xs text-purple-600 font-medium">Power</p>
                    </div>
                    <p className="text-sm sm:text-lg font-bold text-purple-800">{station.power}kW</p>
                  </div>
                </div>

                {/* Status Information */}
                <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-3 pt-2 border-t border-gray-100">
                  <div className="flex items-center gap-2 sm:gap-4 text-sm min-w-0">
                    <div className="flex items-center gap-1">
                      {station.apiKey === "active" ? (
                        <Wifi className="w-4 h-4 text-green-500 flex-shrink-0" />
                      ) : (
                        <WifiOff className="w-4 h-4 text-red-500 flex-shrink-0" />
                      )}
                      <span className="text-gray-600 text-xs sm:text-sm truncate">API: {station.apiKey}</span>
                    </div>
                    <div className="text-gray-600 text-xs sm:text-sm truncate">Last: {station.lastSync}</div>
                  </div>
                  <div className="flex items-center gap-2 sm:gap-3 flex-shrink-0">
                    <div className="flex items-center gap-1 sm:gap-2">
                      <span className="text-xs sm:text-sm text-gray-600">Relay:</span>
                      <Switch
                        checked={station.relayStatus}
                        onCheckedChange={() => toggleRelay(station.id)}
                        className="data-[state=checked]:bg-green-500"
                      />
                    </div>
                    <Button
                      variant="outline"
                      size="sm"
                      className="bg-white/50 hover:bg-white/80 p-1 sm:p-2"
                      onClick={() => toggleRelay(station.id)}
                    >
                      <Power className="w-3 h-3 sm:w-4 sm:h-4" />
                    </Button>
                  </div>
                </div>

                {/* Additional Info */}
                <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-2 text-xs text-gray-500 bg-gray-50 p-2 rounded">
                  <span className="truncate">Firmware: {station.firmwareVersion}</span>
                  {station.connectedUser && <span className="truncate">User: {station.connectedUser}</span>}
                </div>
              </CardContent>
            </Card>
          ))}
        </div>

        {filteredStations.length === 0 && (
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-6 sm:p-8 text-center">
              <Zap className="w-8 h-8 sm:w-12 sm:h-12 text-gray-400 mx-auto mb-4" />
              <h3 className="text-base sm:text-lg font-semibold text-gray-800 mb-2">No stations found</h3>
              <p className="text-sm sm:text-base text-gray-600">Try adjusting your search or filter criteria.</p>
            </CardContent>
          </Card>
        )}
      </div>
    </div>
  )
}
