"use client"

import { useState } from "react"
import { Card, CardContent } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Shield, AlertTriangle, Search, Filter, Download, Eye, Clock, MapPin, Activity } from "lucide-react"
import { SharedNavigation } from "@/components/shared-navigation"

const threatData = [
  {
    id: "THR001",
    timestamp: "2024-01-15 14:30:25",
    type: "MITM Attack",
    station: "ST002",
    stationName: "Mall Parking",
    severity: "high",
    status: "blocked",
    description: "Suspicious network traffic detected between charger and backend",
    source: "192.168.1.45",
    action: "Connection terminated, station isolated",
    mlConfidence: 97.8,
  },
  {
    id: "THR002",
    timestamp: "2024-01-15 14:25:12",
    type: "Voltage Spike",
    station: "ST002",
    stationName: "Mall Parking",
    severity: "medium",
    status: "mitigated",
    description: "Abnormal voltage reading detected: 265V (normal: 240V)",
    source: "Hardware Sensor",
    action: "Relay switched off, maintenance alert sent",
    mlConfidence: 89.2,
  },
  {
    id: "THR003",
    timestamp: "2024-01-15 13:45:33",
    type: "Firmware Tampering",
    station: "ST001",
    stationName: "Downtown Plaza",
    severity: "low",
    status: "monitored",
    description: "Checksum mismatch in firmware verification",
    source: "Integrity Check",
    action: "Continuous monitoring enabled",
    mlConfidence: 76.5,
  },
  {
    id: "THR004",
    timestamp: "2024-01-15 12:15:44",
    type: "Unauthorized Access",
    station: "ST004",
    stationName: "University Campus",
    severity: "high",
    status: "blocked",
    description: "Multiple failed authentication attempts from unknown device",
    source: "10.0.0.123",
    action: "IP blocked, security team notified",
    mlConfidence: 94.1,
  },
  {
    id: "THR005",
    timestamp: "2024-01-15 11:30:17",
    type: "Temperature Anomaly",
    station: "ST005",
    stationName: "Business Park",
    severity: "medium",
    status: "resolved",
    description: "Internal temperature exceeded safe threshold: 45°C",
    source: "Temperature Sensor",
    action: "Cooling system activated, normal operation resumed",
    mlConfidence: 82.7,
  },
]

export default function ThreatsPage() {
  const [threats] = useState(threatData)
  const [searchTerm, setSearchTerm] = useState("")
  const [filterSeverity, setFilterSeverity] = useState("all")
  const [filterStatus, setFilterStatus] = useState("all")

  const filteredThreats = threats.filter((threat) => {
    const matchesSearch =
      threat.type.toLowerCase().includes(searchTerm.toLowerCase()) ||
      threat.station.toLowerCase().includes(searchTerm.toLowerCase()) ||
      threat.stationName.toLowerCase().includes(searchTerm.toLowerCase())
    const matchesSeverity = filterSeverity === "all" || threat.severity === filterSeverity
    const matchesStatus = filterStatus === "all" || threat.status === filterStatus
    return matchesSearch && matchesSeverity && matchesStatus
  })

  const getSeverityColor = (severity: string) => {
    switch (severity) {
      case "high":
        return "bg-red-100 text-red-800 border-red-200"
      case "medium":
        return "bg-orange-100 text-orange-800 border-orange-200"
      case "low":
        return "bg-yellow-100 text-yellow-800 border-yellow-200"
      default:
        return "bg-gray-100 text-gray-800 border-gray-200"
    }
  }

  const getStatusColor = (status: string) => {
    switch (status) {
      case "blocked":
        return "bg-red-100 text-red-800 border-red-200"
      case "mitigated":
        return "bg-blue-100 text-blue-800 border-blue-200"
      case "monitored":
        return "bg-yellow-100 text-yellow-800 border-yellow-200"
      case "resolved":
        return "bg-green-100 text-green-800 border-green-200"
      default:
        return "bg-gray-100 text-gray-800 border-gray-200"
    }
  }

  const getSeverityIcon = (severity: string) => {
    switch (severity) {
      case "high":
        return <AlertTriangle className="w-5 h-5 text-red-500" />
      case "medium":
        return <AlertTriangle className="w-5 h-5 text-orange-500" />
      case "low":
        return <AlertTriangle className="w-5 h-5 text-yellow-500" />
      default:
        return <AlertTriangle className="w-5 h-5 text-gray-500" />
    }
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-purple-50 to-green-50">
      <SharedNavigation currentPage="threats" />

      <div className="lg:ml-64 p-6 space-y-6">
        {/* Header */}
        <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
          <div>
            <h1 className="text-3xl font-bold text-gray-800 flex items-center gap-2">
              <Shield className="w-8 h-8 text-red-600" />
              Threat Logs
            </h1>
            <p className="text-gray-600 mt-1">Monitor and analyze security threats across your network</p>
          </div>
          <div className="flex items-center gap-2">
            <Button variant="outline" size="sm" className="bg-white/80">
              <Download className="w-4 h-4 mr-2" />
              Export CSV
            </Button>
            <Button size="sm" className="bg-gradient-to-r from-red-500 to-orange-500 text-white">
              <Activity className="w-4 h-4 mr-2" />
              Live Monitor
            </Button>
          </div>
        </div>

        {/* Stats Cards */}
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">Total Threats</p>
                  <p className="text-2xl font-bold text-gray-800">{threats.length}</p>
                </div>
                <div className="w-12 h-12 bg-red-100 rounded-lg flex items-center justify-center">
                  <Shield className="w-6 h-6 text-red-600" />
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">High Severity</p>
                  <p className="text-2xl font-bold text-red-600">
                    {threats.filter((t) => t.severity === "high").length}
                  </p>
                </div>
                <div className="w-12 h-12 bg-red-100 rounded-lg flex items-center justify-center">
                  <AlertTriangle className="w-6 h-6 text-red-600" />
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">Blocked</p>
                  <p className="text-2xl font-bold text-orange-600">
                    {threats.filter((t) => t.status === "blocked").length}
                  </p>
                </div>
                <div className="w-12 h-12 bg-orange-100 rounded-lg flex items-center justify-center">
                  <Shield className="w-6 h-6 text-orange-600" />
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-4">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-sm text-gray-600">Resolved</p>
                  <p className="text-2xl font-bold text-green-600">
                    {threats.filter((t) => t.status === "resolved").length}
                  </p>
                </div>
                <div className="w-12 h-12 bg-green-100 rounded-lg flex items-center justify-center">
                  <Activity className="w-6 h-6 text-green-600" />
                </div>
              </div>
            </CardContent>
          </Card>
        </div>

        {/* Search and Filter */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardContent className="p-4">
            <div className="flex flex-col sm:flex-row gap-4">
              <div className="relative flex-1">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 w-4 h-4 text-gray-400" />
                <Input
                  placeholder="Search threats by type, station, or description..."
                  value={searchTerm}
                  onChange={(e) => setSearchTerm(e.target.value)}
                  className="pl-10 bg-white/50"
                />
              </div>
              <div className="flex items-center gap-2">
                <Filter className="w-4 h-4 text-gray-500" />
                <select
                  value={filterSeverity}
                  onChange={(e) => setFilterSeverity(e.target.value)}
                  className="px-3 py-2 bg-white/50 border border-gray-200 rounded-lg text-sm"
                >
                  <option value="all">All Severity</option>
                  <option value="high">High</option>
                  <option value="medium">Medium</option>
                  <option value="low">Low</option>
                </select>
                <select
                  value={filterStatus}
                  onChange={(e) => setFilterStatus(e.target.value)}
                  className="px-3 py-2 bg-white/50 border border-gray-200 rounded-lg text-sm"
                >
                  <option value="all">All Status</option>
                  <option value="blocked">Blocked</option>
                  <option value="mitigated">Mitigated</option>
                  <option value="monitored">Monitored</option>
                  <option value="resolved">Resolved</option>
                </select>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Threats List */}
        <div className="space-y-4">
          {filteredThreats.map((threat) => (
            <Card
              key={threat.id}
              className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg hover:shadow-xl transition-all duration-300"
            >
              <CardContent className="p-6">
                <div className="flex flex-col lg:flex-row lg:items-center lg:justify-between gap-4">
                  <div className="flex items-start gap-4">
                    <div className="flex-shrink-0">{getSeverityIcon(threat.severity)}</div>
                    <div className="flex-1 min-w-0">
                      <div className="flex items-center gap-2 mb-2">
                        <h3 className="text-lg font-semibold text-gray-800">{threat.type}</h3>
                        <Badge className={`${getSeverityColor(threat.severity)} border text-xs`}>
                          {threat.severity.toUpperCase()}
                        </Badge>
                        <Badge className={`${getStatusColor(threat.status)} border text-xs`}>
                          {threat.status.toUpperCase()}
                        </Badge>
                      </div>
                      <p className="text-gray-600 mb-3">{threat.description}</p>
                      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3 text-sm">
                        <div className="flex items-center gap-1 text-gray-500">
                          <Clock className="w-4 h-4" />
                          <span>{threat.timestamp}</span>
                        </div>
                        <div className="flex items-center gap-1 text-gray-500">
                          <MapPin className="w-4 h-4" />
                          <span>
                            {threat.station} - {threat.stationName}
                          </span>
                        </div>
                        <div className="flex items-center gap-1 text-gray-500">
                          <Activity className="w-4 h-4" />
                          <span>ML Confidence: {threat.mlConfidence}%</span>
                        </div>
                        <div className="flex items-center gap-1 text-gray-500">
                          <span>Source: {threat.source}</span>
                        </div>
                      </div>
                    </div>
                  </div>
                  <div className="flex items-center gap-2">
                    <Button variant="outline" size="sm" className="bg-white/50">
                      <Eye className="w-4 h-4 mr-2" />
                      Details
                    </Button>
                  </div>
                </div>
                <div className="mt-4 p-3 bg-gray-50 rounded-lg">
                  <p className="text-sm text-gray-700">
                    <strong>Action Taken:</strong> {threat.action}
                  </p>
                </div>
              </CardContent>
            </Card>
          ))}
        </div>

        {filteredThreats.length === 0 && (
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardContent className="p-8 text-center">
              <Shield className="w-12 h-12 text-gray-400 mx-auto mb-4" />
              <h3 className="text-lg font-semibold text-gray-800 mb-2">No threats found</h3>
              <p className="text-gray-600">Try adjusting your search or filter criteria.</p>
            </CardContent>
          </Card>
        )}
      </div>
    </div>
  )
}
